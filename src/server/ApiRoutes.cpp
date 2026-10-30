/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

 #include "ApiRoutes.hpp"

void ApiRoutes::register_routes(crow::SimpleApp& app) {

    CROW_ROUTE(app, "/api/bots/register")
    .methods(crow::HTTPMethod::POST)([this](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body || !body.has("name")) {
            return crow::response(400, "Name invalid or taken");
        }
        
        std::string name = body["name"].s();
        auto key = registry.register_bot(name);

        if (!key) {
            return crow::response(400, "Key Invalid");
        }

        auto bot = registry.authenticate(*key);

        crow::json::wvalue reply;
        reply["bot_id"] = bot->id;
        reply["api_key"] = *key;

        return crow::response(201, reply);
    });

    CROW_ROUTE(app, "/api/games")
    .methods(crow::HTTPMethod::POST)([this](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!is_admin(req)) {
            return crow::response(403, "Not admin");
        }
        if (!body || !body.has("white_id") || !body.has("black_id")) {
            return crow::response(400, "Missing or Invalid White/Black ID");
        }
        std::string white_id = body["white_id"].s();
        std::string black_id = body["black_id"].s();

        auto game = gamecontroller.start_game(white_id, black_id);
        
        if (!game) {
            return crow::response(400, "Can't start game");
        }

        crow::json::wvalue reply;
        reply["game_id"] = *game;
        return crow::response(201, reply);

    });

    CROW_ROUTE(app, "/api/games")
    .methods(crow::HTTPMethod::GET)([this](const crow::request&){
        crow::json::wvalue::list result;
        std::vector<GameState> all_games = gamecontroller.all_games();

        for (auto& state : all_games) {
            crow::json::wvalue entry;
            entry["game_id"] = state.game_id;
            entry["white_id"] = state.white_id;
            entry["black_id"] = state.black_id;
            entry["status"] = state.status;
            result.push_back(std::move(entry));
        }

        crow::json::wvalue reply;
        reply["games"] = std::move(result);
        return crow::response(200, reply);
    });

    CROW_ROUTE(app, "/api/games/<string>")
    .methods(crow::HTTPMethod::GET)([this](const crow::request& req, std::string game_id){
        std::string key = req.get_header_value("X-API-KEY");
        return to_response(gamecontroller.get_state(game_id, key));
    });

    CROW_ROUTE(app, "/api/games/<string>/watch")
    .methods(crow::HTTPMethod::GET)([this](const crow::request&, std::string game_id){
        GameState state = gamecontroller.spectate(game_id);
        if (!state.ok) {
            return crow::response(404, "Cannot watch");
        }

        crow::json::wvalue reply;
        reply["game_id"] = state.game_id;
        reply["white_id"] = state.white_id;
        reply["black_id"] = state.black_id;
        reply["fen"] = state.fen;
        reply["status"] = state.status;
        reply["moves"] = state.moves;

        return crow::response(200, reply);
    });

    CROW_ROUTE(app, "/api/games/<string>/move")
    .methods(crow::HTTPMethod::POST)([this](const crow::request& req, std::string game_id){
        auto body = crow::json::load(req.body);
        if (!body || !body.has("move")) {
            return crow::response(400, "Body not valid & doesn't have a move");
        }

        std::string key = req.get_header_value("X-API-KEY");
        std::string move = body["move"].s();
        return to_response(gamecontroller.make_move(game_id, key, move));
    });
        
    CROW_ROUTE(app, "/api/leaderboard")
    .methods(crow::HTTPMethod::GET)([this](const crow::request& req){
        std::vector<Leaderboard::Rating> leader_top = leaderboard.top(10);
        crow::json::wvalue::list leader;
        crow::json::wvalue reply;
        
        for (const auto& rating : leader_top) {
            crow::json::wvalue entry;
            entry["bot_id"] = rating.bot_id;
            entry["wins"] = rating.wins;
            entry["losses"] = rating.losses;
            entry["draws"] = rating.draws;
            entry["rating"] = std::lround(rating.rating);
            leader.push_back(std::move(entry));
        }

        reply["leaderboard"] = std::move(leader);
        return crow::response(200, reply);

    });

    CROW_ROUTE(app, "/api/my-games")
    .methods(crow::HTTPMethod::GET)([this](const crow::request& req){
        std::string key = req.get_header_value("X-API-KEY");
        auto bot = registry.authenticate(key);
        if (!bot) {
            return crow::response(401, "No auth key");
        }

        std::vector<std::string> ids = gamecontroller.games_for_bot(bot->id);
        crow::json::wvalue::list result;
        crow::json::wvalue reply;
        for (const auto& id : ids) {
            result.push_back(id);
        }

        reply["games"] = std::move(result);
        return crow::response(200, reply);

    });

    CROW_ROUTE(app, "/api/tournament")
    .methods(crow::HTTPMethod::GET)([this](const crow::request& req){
        crow::json::wvalue reply;
        crow::json::wvalue::list result;

        reply["round"] = manager.round_number();
        reply["total_rounds"] = manager.total_rounds();

        std::vector<TournamentManager::Pairing> pair = manager.current_round();

        for (auto& pairing : pair) {
            crow::json::wvalue entry;
            entry["white_id"] = pairing.white_id;
            entry["black_id"] = pairing.black_id;
            if (!pairing.game_id) {
                entry["game_id"] = nullptr;
            } else {
                entry["game_id"] = *pairing.game_id;
            }

            result.push_back(std::move(entry));
        }

        reply["pairings"] = std::move(result);
        return crow::response(200, reply);
    });

    CROW_ROUTE(app, "/api/tournament/start")
    .methods(crow::HTTPMethod::POST)([this](const crow::request& req){
        auto body = crow::json::load(req.body);
        crow::json::wvalue reply;
        // TournamentManager manager;
        if (!is_admin(req)) {
            return crow::response(403, "Not admin");
        }

        if (manager.create_schedule() == false) {
            return crow::response(400, "Need 2 registered bots");
        }
        manager.start_round();
        reply["round"] = manager.round_number();
        reply["total_rounds"] = manager.total_rounds();
        return crow::response(200, reply);
    });

    CROW_ROUTE(app, "/api/tournament/next")
    .methods(crow::HTTPMethod::POST)([this](const crow::request& req){
        auto body = crow::json::load(req.body);
        crow::json::wvalue reply;
        // TournamentManager manager;
        if (!is_admin(req)) {
            return crow::response(403, "Not admin");
        }

        if (manager.round_finished() == false) {
            return crow::response(409, "Current round still in progress");
        }

        if (manager.next_round() == false) {
            reply["finished"] = true;
            return crow::response(200, reply);
        }
        reply["rounds"] = std::lround(manager.round_number());
        reply["total_rounds"] = manager.total_rounds();
        return crow::response(200, reply);
    });
}

crow::response ApiRoutes::to_response(const GameState& gamestate) const {
    crow::json::wvalue body;
    if (gamestate.ok) {
        body["ok"] = true;
        body["game_id"] = gamestate.game_id;
        body["fen"] = gamestate.fen;
        body["your_colour"] = gamestate.your_colour;
        body["status"] = gamestate.status;
        crow::json::wvalue::list moves;
        for (auto& move : gamestate.moves) {
            moves.push_back(std::move(move));
        }
        body["moves"] = std::move(moves);
    }
    else {
        body["ok"] = false;
        body["error"] = gamestate.message;
    }

    int code = 200;
    switch (gamestate.error) {
        case  GameState::BAD_KEY:
            code = 401;
            break;
        case GameState::GAME_NOT_FOUND:
            code = 404;
            break;
        case GameState::NOT_A_PLAYER:
            code = 403;
            break;
        case GameState::NOT_YOUR_TURN:
            code = 403;
            break;
        case GameState::ILLEGAL_MOVE:
            code = 409;
            break;
        case GameState::GAME_OVER:
            code = 409;
            break;
        default:
            break;
    }
    return crow::response(code, body);
}

bool ApiRoutes::is_admin(const crow::request& req) const {
    return req.get_header_value("X-ADMIN-KEY") == admin_key;
}

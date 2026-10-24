/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

 #include "ApiRoutes.hpp"

void ApiRoutes::register_routes(crow::SimpleApp& app) {
    // auto body = crow::json::load(req.body);
    // if (body && body.has("name")) {
    //     body["name"].s();
    // }

    // req.get_header_value("X-API-KEY");

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
    .methods(crow::HTTPMethod::POST)([](const crow::request& req){
        auto body = crow::json::load(req.body);
    });

    CROW_ROUTE(app, "/api/games/<string>")
    .methods(crow::HTTPMethod::GET)([](const crow::request& req){
        auto body = crow::json::load(req.body);
    });

    CROW_ROUTE(app, "/api/games/<string>/move")
    .methods(crow::HTTPMethod::POST)([](const crow::request& req){
        auto body = crow::json::load(req.body);
    });
        
    CROW_ROUTE(app, "/api/leaderboard")
    .methods(crow::HTTPMethod::GET)([](const crow::request& req){
        auto body = crow::json::load(req.body);
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
        for (const auto& move : moves) {
            body["moves"] = std::move(moves);
        }
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
    }
    return crow::response(code, body);
}


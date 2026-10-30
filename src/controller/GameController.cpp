/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

 #include "GameController.hpp"


std::optional<std::string> GameController::start_game(const std::string& white_id, const std::string& black_id){
    if (!registry.get(white_id) || !registry.get(black_id) || white_id == black_id) {
        return std::nullopt;
    }

    std::lock_guard<std::mutex> guard(games_lock);
    std::string id = "game-" + std::to_string(counter);
    counter += 1;

    games[id] = std::make_unique<Match>();
    games[id]->black_id = black_id;
    games[id]->white_id = white_id;

    return id;
}

GameState GameController::get_state(const std::string& game_id, const std::string& api_key) {
    GameState gamestate;
    std::optional<Bot> bot = registry.authenticate(api_key);
    Match* match = find_match(game_id);
    if (!bot) {
        return make_error(gamestate.BAD_KEY, "Bad API Key");
    }

    if (match == nullptr) {
        return make_error(gamestate.GAME_NOT_FOUND, "Couldn't find a game");
    }

    std::lock_guard<std::mutex> guard(match->lock);

    if (bot->id != match->black_id && bot->id != match->white_id) {
        return make_error(gamestate.NOT_A_PLAYER, "Couldn't find a player");
    }
    return build_state(*match, game_id, *bot);
}

GameState GameController::make_move(const std::string& game_id, const std::string& api_key, const std::string& uci) {
    GameState gamestate;
    Piece::Colour colour;
    std::optional<Bot> bot = registry.authenticate(api_key);
    Match* match = find_match(game_id);
    if (!bot) {
        return make_error(gamestate.BAD_KEY, "Bad API Key");
    }

    if (match == nullptr) {
        return make_error(gamestate.GAME_NOT_FOUND, "Couldn't find a game");
    }

    std::lock_guard<std::mutex> guard(match->lock);

    if (bot->id != match->black_id && bot->id != match->white_id) {
        return make_error(gamestate.NOT_A_PLAYER, "Couldn't find a player");
    }

    if  (match->game.status() != Game::IN_PROGRESS) {
        return make_error(gamestate.GAME_OVER, "Game Over!");
    }

    if (bot->id == match->white_id) {
        colour = Piece::WHITE;
    } else {
        colour = Piece::BLACK;
    }

    if (match->game.side_to_move() != colour) {
        return make_error(gamestate.NOT_YOUR_TURN, "Its not your turn");
    }

    if (!match->game.try_move(uci)) {
        return make_error(gamestate.ILLEGAL_MOVE, "Can't make that move");
    }

    if (match->game.status() != Game::IN_PROGRESS) {
        finish(*match);
    }
    return build_state(*match, game_id, *bot);
}

void GameController::finish(Match& match) {
    Leaderboard::Status result;
    switch (match.game.status()) {
        case Game::BLACK_WINS:
            result = Leaderboard::BLACK_WINS;
            break;
        case Game::WHITE_WINS:
            result = Leaderboard::WHITE_WINS;
            break;
        case Game::DRAW:
            result = Leaderboard::DRAW;
            break;
        case Game::IN_PROGRESS:
            return;
    }
    leaderboard.record(match.white_id, match.black_id, result);
}

GameState GameController::build_state(const Match& match, const std::string& game_id, const Bot& bot) const {
    GameState gamestate;
    gamestate.ok = true;
    gamestate.error = gamestate.NONE;
    gamestate.game_id = game_id;
    gamestate.fen = match.game.to_fen();
    gamestate.moves = match.game.history_uci();
    
    if (bot.id == match.white_id) {
            gamestate.your_colour = "white";
    } 
    else {
        gamestate.your_colour = "black";
    }

    gamestate.status = status_text(match.game.status());
    return gamestate;
}

GameController::Match* GameController::find_match(const std::string& game_id) {
    std::lock_guard<std::mutex> guard(games_lock);
    auto it = games.find(game_id);
    if (it == games.end()) {
        return nullptr;
    }
    return it->second.get();
}

GameState GameController::make_error(GameState::Error error, const std::string& message) {
    GameState gamestate;
    gamestate.ok = false;
    gamestate.error = error;
    gamestate.message = message;
    return gamestate;
}

std::optional<Game::Status> GameController::game_status(const std::string& game_id) {
    if (find_match(game_id) == nullptr) {
        return std::nullopt;
    }
    Match* match = find_match(game_id);
    std::lock_guard<std::mutex> guard(match->lock);
    return match->game.status();
}


std::vector<std::string> GameController::games_for_bot(const std::string& bot_id){
    std::lock_guard<std::mutex> guard(games_lock);
    std::vector<std::string> result;
    for (auto& [id, match] : games) {
        if (match->white_id == bot_id || match->black_id == bot_id) {
            std::lock_guard<std::mutex> match_guard(match->lock);
            if (match->game.status() == Game::IN_PROGRESS) {
                result.push_back(id);
            }
        } 
    }
    return result;
}

GameState GameController::spectate(const std::string& game_id) {
    GameState gamestate;
    Match* match = find_match(game_id);

    if (match == nullptr) {
        return make_error(gamestate.GAME_NOT_FOUND, "Couldn't find a game");
    }

    std::lock_guard<std::mutex> guard(match->lock);
    

    gamestate.ok = true;
    gamestate.fen = match->game.to_fen();
    gamestate.moves = match->game.history_uci();
    gamestate.status = status_text(match->game.status());

    gamestate.game_id = game_id;
    gamestate.white_id = match->white_id;
    gamestate.black_id = match->black_id;

    return gamestate;
}

std::string GameController::status_text(Game::Status status) {

    switch(status) {
        case Game::IN_PROGRESS:
            return "in_progress";
        case Game::WHITE_WINS:
            return "white_wins";
        case Game::BLACK_WINS:
            return "black_wins";
        case Game::DRAW:
            return "draw";
        default:
            break;
    }
    return "unknown";
}


std::vector<GameState> GameController::all_games() {
    std::lock_guard<std::mutex> guard(games_lock);
    std::vector<GameState> result;
    for (auto& [id, match] : games) {
        std::lock_guard<std::mutex> match_guard(match->lock);
        GameState state;
        state.game_id = id;
        state.white_id = match->white_id;
        state.black_id = match->black_id;
        state.status = status_text(match->game.status());
        result.push_back(state);
    }
    return result;
}
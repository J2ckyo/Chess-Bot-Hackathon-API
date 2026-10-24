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

    if (bot->id != match->black_id || bot->id != match->white_id) {
        return make_error(gamestate.NOT_A_PLAYER, "Couldn't find a player");
    }
    return build_state(*match, game_id, *bot);
}

GameState GameController::make_move(const std::string& game_id, const std::string& api_key, const std::string& uci) {

}

void GameController::finish(Match& match) {

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

    switch(match.game.status()) {
        case Game::IN_PROGRESS:
            gamestate.status = "In_Progess";
            break;
        case Game::WHITE_WINS:
            gamestate.status = "White_Wins";
            break;
        case Game::BLACK_WINS:
            gamestate.status = "Black_Wins";
            break;
        case Game::DRAW:
            gamestate.status = "Draw";
            break;
    }
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



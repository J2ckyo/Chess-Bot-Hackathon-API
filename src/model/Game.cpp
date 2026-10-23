#include "Game.hpp"

Game::Game(const std::string& fen) {
    std::optional<Board> result = Board::read_all_fen(fen);

    if (!result) {
        throw std::invalid_argument("Bad FEN");
    }
    else {
        board = *result;
    }

    positions[positions_key()] = 1;
    update_status();
}

bool Game::try_move(const std::string& uci) {
    std::optional<Move> move = Move::from_uci(uci);
    if (!move) {
        return false;
    }
    if (current_status != IN_PROGRESS) {
        return false;
    }
    if (!Move::from_uci(uci)) {
        return false;
    }
    if (!MoveGenerator::is_legal(board, *move)) {
        return false;
    }
    board.apply(*move);
    history.push_back(*move);

    positions[positions_key()] += 1;
    update_status();
    return true;
}

Game::Status Game::status() const {
    return current_status;
}

std::string Game::to_fen() const {
    return board.all_to_fen();
}

std::vector<std::string> Game::history_uci() const {
    std::vector<std::string> history_vec;
    for (const auto& move : history) {
        history_vec.push_back(move.to_uci());
    }
    return history_vec;
}

std::string Game::positions_key() const {
    std::string placement, turn, castling_field, ep_field;
    std::stringstream fields(board.all_to_fen());

    return placement + " " + turn + " " + castling_field + " " + ep_field;
}

void Game::update_status() {
    if (MoveGenerator::legal_moves(board).empty()) {
        Piece::Colour side = board.get_side_to_move();
        if (MoveGenerator::in_check(board, side) == true) {
            if (board.get_side_to_move() == Piece::WHITE) {
                current_status = Status::BLACK_WINS;
            } else {
                current_status = Status::WHITE_WINS;
            }
        } else {
            current_status = Status::DRAW;
        }
    }

    if (board.get_half_move() >= 100) {
        current_status = Status::DRAW;
    }

    if (positions[positions_key()] >= 3) {
        current_status = Status::DRAW;
    }
}

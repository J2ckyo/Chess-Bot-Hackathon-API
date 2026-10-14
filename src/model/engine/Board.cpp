/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#include "Board.hpp"

std::optional<Piece> Board::piece_at(const Square& square) const{
    // Checks if Piece is onboard
    if (square.on_board(square.rank, square.file) == false) {
        return std::nullopt;
    }
    return board[square.rank][square.file];
}


void Board::place(const Square& square, std::optional<Piece> piece){
    // Places a piece on the board. 
    board[square.rank][square.file] = piece;
}


std::optional<Board> Board::read_all_fen(std::string fen_json) {
    std::string token, placement, turn, castling_field, ep_field;
    Board result;
    int rank = 7;
    int file = 0;
    int half = 0;
    int full = 0;
    std::stringstream rows(placement);
    std::stringstream ss(fen_json);
    std::stringstream fields(fen_json);

    // Split into 6 fields
    if (!(fields >> placement >> turn >> castling_field >> ep_field >> half >> full)) {
        return std::nullopt;
    }

    // Pieces
    while (getline(ss, token, '/')) {
        if (rank < 0) return std::nullopt;

        for (auto it = token.begin(); it != token.end(); ++it){
            if (std::isdigit(*it)){
                if (*it < '1' || *it > '8') {
                    return std::nullopt;
                }
                file += *it - '0';
                continue;
            }

            std::optional<Piece> p = Piece::piece_from_fen(*it);

            if (!p) {
                return std::nullopt;
            }
            if (file > 7) {
                return std::nullopt;
            }

            result.place(Square{file, rank}, p);
            file += 1;
        }

        if (file != 8) {
            return std::nullopt;
        }
        rank -= 1;
        file =0;
    }

    if (rank != -1) {
        return std::nullopt; 
    }

    // Turn
    if (turn == "w") {
        result.side_to_move = Piece::WHITE;
    }
    else if (turn == "b") {
        result.side_to_move = Piece::BLACK;
    }
    else {
        return std::nullopt;
    }

    // Castling
    if (castling_field != "-") {
        for (char c : castling_field) {
            if (c != 'K' && c != 'Q' && c != 'k' && c != 'q') {
                return std::nullopt;
            }
        }
    }

    result.castling = castling_field;

    // Enpassant
    if (ep_field == "-") {
        result.enp_square = std::nullopt;
    }
    else {
        std::optional<Square> sq = Square::squares_from_fen(ep_field);
        if (!sq) {
            return std::nullopt;
        }
        result.enp_square = sq;
    }

    // Move Counters
    if (half < 0 || full < 1) {
        return std::nullopt;
    }
    result.halfmove = half;
    result.fullmove = full;

    return result;
}


std::string all_to_fen() {
    
}


void apply(const Move& move){

}



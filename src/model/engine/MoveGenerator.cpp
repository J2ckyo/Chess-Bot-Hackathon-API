/**
 * @author Jack Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#include "MoveGenerator.hpp"

const std::vector<std::pair<int, int>> Knight = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-2, -1}, {-2, 1}, {-1, 2}, {-1, -2}};
const std::vector<std::pair<int, int>> King = {{1, 1}, {-1, -1}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, -1}, {-1, 1}};
const std::vector<std::pair<int, int>> Rook = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
const std::vector<std::pair<int, int>> Bishop = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

bool is_attacked(const Board& board, const Square& square, Piece::Colour by) {

}

bool in_check(const Board& board, Piece::Colour colour) {

}


std::vector<Move> pseudo_legal(const Board& board) {

}

std::vector<Move> legal_moves(const Board& board) {

}

bool is_legal(const Board& board, const Move& move) {

}

std::optional<Square> MoveGenerator::find_king(const Board& board, Piece::Colour colour) {
    std::optional<Piece> king;
    for (int rank = 7; rank >= 0; rank--){
        for (int file = 0; file <= 7; file++) {
            king = board.piece_at(Square{file, rank});
            if (king && king->kind == Piece::KING && king->colour == colour) {
                return Square{file, rank};
            }
        }
    }
    return std::nullopt;
}


void jump_moves(const Board& board, Square& square, Piece::Colour colour, const std::vector<std::pair<int, int>>&, std::vector<Move>&) {

}

void slide_moves(const Board& board, Square& square, Piece::Colour colour, const std::vector<std::pair<int, int>>&, std::vector<Move>&) {

}




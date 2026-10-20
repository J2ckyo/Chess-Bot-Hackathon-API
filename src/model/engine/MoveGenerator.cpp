/**
 * @author Jack Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#include "MoveGenerator.hpp"

const std::vector<std::pair<int, int>> Knight = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-2, -1}, {-2, 1}, {-1, 2}, {-1, -2}};
const std::vector<std::pair<int, int>> King = {{1, 1}, {-1, -1}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, -1}, {-1, 1}};
const std::vector<std::pair<int, int>> Rook = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
const std::vector<std::pair<int, int>> Bishop = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

bool MoveGenerator::is_attacked(const Board& board, const Square& square, Piece::Colour by) {

}

bool MoveGenerator::in_check(const Board& board, Piece::Colour colour) {

}


std::vector<Move> MoveGenerator::pseudo_legal(const Board& board) {

}

std::vector<Move> MoveGenerator::legal_moves(const Board& board) {

}

bool MoveGenerator::is_legal(const Board& board, const Move& move) {

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


void MoveGenerator::jump_moves(const Board& board, const Square& from, Piece::Colour colour, const std::vector<std::pair<int, int>>& offset, std::vector<Move>& moves) {
    for (const std::pair<int, int>& offset : offset) {
        int new_file = from.file + offset.first;
        int new_rank = from.rank + offset.second;

        if (!Square::on_board(new_rank, new_file)) {
            continue;
        }

        Square target{new_file, new_rank};
        std::optional<Piece> occupant = board.piece_at(target);

        if (occupant && occupant->colour == colour) {
            continue;
        }

        moves.push_back(Move{from, target, std::nullopt});
    }
}

void MoveGenerator::slide_moves(const Board& board, const Square& from, Piece::Colour colour, const std::vector<std::pair<int, int>>& directions, std::vector<Move>& moves) {
    for (const std::pair<int, int>& dir : directions) {
        int new_file = from.file;
        int new_rank = from.rank;

        while (true) {
            new_file += dir.first;
            new_rank += dir.second;

            if (!Square::on_board(new_file, new_rank)) {
                break;
            }

            Square target{new_file, new_rank};
            std::optional<Piece> occupant = board.piece_at(target);

            if (!occupant) {
                moves.push_back(Move{from, target, std::nullopt});
            }
            else {
                if (occupant->colour != colour) {
                    moves.push_back(Move{from, target, std::nullopt});
                }
                break;
            }
        }
    }
}




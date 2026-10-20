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
    for (const auto& move : Knight) {
        int new_file = square.file + move.first;
        int new_rank = square.rank + move.second;
        bool is_knight = Square::on_board(new_file, new_rank);
        std::optional<Piece> knight = board.piece_at(Square{new_file, new_rank});
        if (is_knight && by == knight->colour && knight->colour == Piece::KNIGHT) {
            return true;
        }
    }

    for (const auto& move : King) {
        int new_file = square.file + move.first;
        int new_rank = square.rank + move.second;
        bool is_king = Square::on_board(new_file, new_rank);
        std::optional<Piece> king = board.piece_at(Square{new_file, new_rank});
        if (is_king && by == king->colour && king->colour == Piece::KING) {
            return true;
        } 
    }

    for (const auto& move : Rook) {
        int new_file = square.file;
        int new_rank = square.rank;
        while (true) {
            new_file += move.first;
            new_rank += move.second;

            if (!Square::on_board(new_file, new_rank)) {
                break;
            }

            std::optional<Piece> occupant = board.piece_at(Square{new_file, new_rank});
            if (occupant) {
                if ((occupant->kind == Piece::ROOK || occupant->kind == Piece::QUEEN) && occupant->colour == by) {
                    return true;
                }
                break;
            }
        }
    }

    for (const auto& move : Bishop) {
        int new_file = square.file;
        int new_rank = square.rank;
        while (true) {
            new_file += move.first;
            new_rank += move.second;

            if (!Square::on_board(new_file, new_rank)) {
                break;
            }

            std::optional<Piece> occupant = board.piece_at(Square{new_file, new_rank});
            if (occupant) {
                if ((occupant->kind == Piece::BISHOP || occupant->kind == Piece::QUEEN) && occupant->colour == by) {
                    return true;
                }
                break;
            }
        }
    }

    int pawn_rank = (by == Piece::WHITE) ? square.rank - 1 : square.rank + 1;

    for (int file_change : {-1, 1}) {
        int pawn_file = square.file + file_change;

        if (!Square::on_board(pawn_file, pawn_rank)) {
            continue;
        }

        std::optional<Piece> occupant = board.piece_at(Square{pawn_file, pawn_rank});
        if (occupant && occupant->kind == Piece::PAWN && occupant->colour == by) {
            return true;
        }
    }

    return false;
}

bool MoveGenerator::in_check(const Board& board, Piece::Colour colour) {
    std::optional<Square> is_king = find_king(board, colour);
    if (!is_king) {
        return false;
    }

    return is_attacked(board, *is_king, Piece::opposite(colour));
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


void MoveGenerator::pawn_moves(const Board& board, const Square& from, Piece::Colour colour, std::vector<Move>& moves) {
    int direction = (colour == Piece::WHITE) ? 1 : -1;
    int start_rank = (colour == Piece::WHITE) ? 1 : 6;
    int last_rank = (colour == Piece::WHITE) ? 7 : 0;

    auto add_pawn_move = [&](const Square& to) {
        if (to.rank == last_rank) {
            moves.push_back(Move{from, to, Piece::QUEEN});
            moves.push_back(Move{from, to, Piece::ROOK});
            moves.push_back(Move{from, to, Piece::BISHOP});
            moves.push_back(Move{from, to, Piece::KNIGHT});
        } 
        else {
            moves.push_back(Move{from, to, std::nullopt});
        }
    };

    int forward_rank = from.rank + direction;

    if (Square::on_board(forward_rank, from.file)) {
        Square one{from.file, forward_rank};
        if (!board.piece_at(one)) {
            add_pawn_move(one);

            int two_rank = from.rank + 2 * direction;
            if (from.rank == start_rank) {
                Square two{from.file, two_rank};
                if (!board.piece_at(two)) {
                    moves.push_back(Move{from, two, std::nullopt});
                }
            }
        }
    }

    std::optional<Square> ep = board.get_en_passant();

    
}




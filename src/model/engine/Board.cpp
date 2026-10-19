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
    std::stringstream fields(fen_json);

    // Split into 6 fields
    if (!(fields >> placement >> turn >> castling_field >> ep_field >> half >> full)) {
        return std::nullopt;
    }

    std::stringstream rows(placement);

    // Pieces
    while (getline(rows, token, '/')) {
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


std::string Board::all_to_fen() const{
    // Reverse of From_Fen
    // Take board[rank][file] or piece_at, which gives std::optional<Piece>
    // Then do side_to_move, castling, enp_square, halfmove, fullmove
    std::string fen;
    int counter = 0;
    for (int rank = 7; rank >= 0; rank--){
        for (int file = 0; file <= 7; file++) {
            std::optional<Piece> p = piece_at(Square{file, rank});
            // Add to counter if no piece at square
            if (p == std::nullopt) {
                counter += 1;
            }
            
            else {
                // Adds counter to Fen string
                if (counter > 0) {
                    fen = fen + std::to_string(counter);
                }
                fen = fen + Piece::piece_to_fen(p->kind, p->colour);
                counter = 0;
            }
        }
        if (counter > 0) {
            fen  = fen + std::to_string(counter);
            counter = 0;
        }

        if (rank != 0) {
            fen += '/';
        }
    }

    // Turn
    // Add space then colour
    fen += " ";
    if (side_to_move == Piece::WHITE) {
        fen += "w";
    }
    else {
        fen += "b";
    }

    // Add castling to fen
    fen += " ";
    fen += castling;

    // Adding Enpassant to fen
    fen += " ";
    if (enp_square != std::nullopt) {
        fen += enp_square->squares_to_fen();
    }
    else {
        fen += "-";
    }

    // Adding half moves
    fen += " ";
    fen += std::to_string(halfmove);
    fen += " ";
    fen += std::to_string(fullmove);

    return fen;
}


void Board::apply(const Move& move){
    // Apply a normal move.
    std::optional<Piece> piece = piece_at(move.from);
    std::optional<Square> old_ep = enp_square;
    bool capture = false;
    if (piece_at(move.to)) {
        capture = true;
    }
    // Places Piece on square
    place(move.to, piece);
    // Returns from location to empty
    place(move.from, std::nullopt);

    // If pawn moved or captured true reset to 0
    if (piece->kind == Piece::PAWN || capture == true) {
        halfmove = 0;
    }
    else {
        halfmove += 1;
    }

    // If black moved add 1
    if (side_to_move == Piece::BLACK) {
        fullmove += 1;
    } 

    // Flip turn
    side_to_move = Piece::opposite(side_to_move);

    // Apply Special Moves
    // Pawn Promotion
    if (move.promotion_piece) {
        std::optional<Piece> prom_piece = Piece{*move.promotion_piece, piece->colour};
        place(move.to, prom_piece);

    }

    // Enpassant Moves
    enp_square = std::nullopt;
    if ((piece->kind == Piece::PAWN) && abs(move.to.rank - move.from.rank) == 2) {
        // clear square
        enp_square = Square{move.from.file, (move.from.rank + move.to.rank) / 2};
    }

    // Enpassant Capture
    if ((piece->kind == Piece::PAWN) && (move.from.file != move.to.file) && (old_ep && move.to == *old_ep)) {
        place(Square{move.to.file, move.from.rank}, std::nullopt);
    }

    // Castling
    if ((piece->kind == Piece::KING) && abs(move.to.file - move.from.file) == 2) {
        int r = move.from.rank;
        Square rook_from{0, r};
        Square rook_to{0, r};

        if (move.to.file == 6) {
            rook_from = Square{7, r};
            rook_to = Square{5, r};
        }
        else {
            rook_from = Square{0, r};
            rook_to = Square{3, r};
        }

        place(rook_to, piece_at(rook_from));
        place(rook_from, std::nullopt);
    }

    // Castling Rights
    auto remove_right = [this](char c) {
        std::size_t pos = castling.find(c);
        if (pos != std::string::npos) castling.erase(pos, 1);
    };

    const Square e1{4, 0}, e8{4, 7};
    const Square a1{0, 0}, h1{7, 0}, a8{0, 7}, h8{7, 7};

    if (move.from == e1) {remove_right('K'); remove_right('Q'); }
    if (move.from == e8) {remove_right('k'); remove_right('q'); }

    if (move.from == a1 || move.to == a1) remove_right('Q');
    if (move.from == h1 || move.to == h1) remove_right('K');
    if (move.from == a8 || move.to == a8) remove_right('q');
    if (move.from == h8 || move.to == h8) remove_right('k');

    if (castling.empty()) castling = "-";

}



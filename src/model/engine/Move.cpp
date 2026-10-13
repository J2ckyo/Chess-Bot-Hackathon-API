/**
 * @author Jack Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

 #include "Move.hpp"

std::optional<Move>  Move::from_uci(std::string uci_string){

    std::optional<Piece::Kind> promotion_piece;
    // Checks if string is length 4 or 5
    if (uci_string.length() >= 6 || uci_string.length() <= 3){
        return std::nullopt;
    }
    if (uci_string.length() == 5) {
        // Knight, Rook, Bishop, Queen
        std::string types = "nrbq";
        // Checks if promotion piece is a piece type
        if (!types.contains(uci_string[4])) {
            return std::nullopt;
        }
        promotion_piece = Piece::piece_from_fen(uci_string[4])->kind;
    }
    // passes uci_string and converts the whole thing into a move format the server can use
    std::optional<Square> from = Square::squares_from_fen(uci_string.substr(0, 2));
    std::optional<Square> to = Square::squares_from_fen(uci_string.substr(2, 2));

    if (!from || !to) {
        return std::nullopt;
    }

    return Move{*from, *to, promotion_piece};
 
}

std::string Move::to_uci() const{
    // Pass squares into Squares_to_fen to convert back to UCI

    std::string uci = from.squares_to_fen() + to.squares_to_fen();
    if (promotion_piece != std::nullopt) {
      uci =  uci + Piece::piece_to_fen(*promotion_piece, Piece::BLACK);
    }
    return uci;
}


 
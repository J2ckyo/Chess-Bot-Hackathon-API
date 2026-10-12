/**
 * @author Jack Simonton. <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#include "Piece.hpp"
#include <cctype>

char Piece::piece_to_fen(Kind kind, Colour colour) const{
    std::string Pieces = "prnbqk";
    char letter;
    // Converts the ENUM kind to type int which is the index
    int piece_index = static_cast<int>(kind);
    // Assigns letter then FEN Char
    char letter = Pieces[piece_index];
    // Assigns what colour the piece belongs to
    letter = colour == WHITE ? toupper(letter) : tolower(letter);

    return letter;
}

std::optional<Piece> Piece::piece_from_fen(char letter) {
    std::string Black_Pieces = "prnbqk";
    std::string White_Pieces = "PRNBQK";
    //FEN peice case
    if (White_Pieces.contains(letter) || Black_Pieces.contains(letter)){
        // Sets Colour. IF letter is Uppercase It's WHITE else BLACK
        Colour piece_colour = std::isupper(letter) ? WHITE : BLACK;
        // Find index of letter. Same for both colours.
        // Will break if order in ENUM or Strings change
        int piece_index = White_Pieces.find(toupper(letter));
        Kind piece_kind = static_cast<Kind>(piece_index);
        return Piece{piece_kind, piece_colour};
    }
    // nullptr case
    return std::nullopt;
}

/**
 * @brief IF colour == WHITE, then return BLACK,
 * else return WHITE.
 */
Piece::Colour Piece::opposite(Colour colour){
  return colour == WHITE ? BLACK : WHITE;
}










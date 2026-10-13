/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#ifndef PIECE_HPP
#define PIECE_HPP

#include <string>
#include <optional>

/**
 * @class Piece Class
 * @brief Kind/Colour of Pieces
 * and helper functions
 */

class Piece{
    public:

        /**
         * @brief Enum of kinds of pieces
         */ 
        enum Kind{
            PAWN,
            ROOK,
            KNIGHT,
            BISHOP,
            QUEEN,
            KING
        };

        /**
         * @brief Enum of kinds of Colours
         */
        enum Colour{
            WHITE,
            BLACK
        };

        /**
         * @brief Converts Letters K, q, .. , 
         * to King BLACK, Queen WHITE ...
         */
        static std::optional<Piece> piece_from_fen(char letter);

        /**
         * @brief Converts pieces King, Queen, ..
         * to K, q, .. 
         */
        static char piece_to_fen(Kind kind, Colour colour);

        /**
         * @brief Changes one colour for another.
         * White <-> Black
         */
        Colour opposite(Colour);
        
        Kind kind;
        Colour colour;
};


#endif

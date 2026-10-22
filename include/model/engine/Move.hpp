/**
 * @author Jack Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#ifndef MOVE_HPP
#define MOVE_HPP

#include <string>
#include <optional>
#include "Square.hpp"
#include "Piece.hpp"

class Move{
    public:
        /**
         * @brief Data field that holds starting square
         * 
         */
        Square from;

        /**
         * @brief Data field that holds ending square
         * 
         */
        Square to;

        /**
         * @brief Field that holds a piece being promated.
         *  
         */
        std::optional<Piece::Kind> promotion_piece;

        /** 
         * @brief Creates a Move object from a UCI string
         * 
         * @param uci_string The UCI string representing the move
         * @return std::optional<Move> The created Move object
         */
        static std::optional<Move> from_uci(std::string uci_string);

        /**
         * @brief converts the Move object to a UCI string representation
         * 
         * @return std::string 
         */
        std::string to_uci() const;

        /**
         * @brief Comparison Constructor
         * 
         */
        bool operator==(const Move&) const = default;
};

#endif

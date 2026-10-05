/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#ifndef PIECE_HPP
#define PIECE_HPP

#include <vector>
#include <memory>
#include "Types.hpp"

// Forward Declaration
class Board;
class MoveRule;

/**
 * @brief Replaces inheritance with delegration
 * This is the context in the strategy pattern
 */

class Piece{
    public:

        /**
         * @brief Piece Constructor
         */ 
        Piece(PieceKind kind, Colour colour, std::shared_ptr<const MoveRule> rule);

        /**
         * @brief Return Moves method
         */

        std::vector<Position> moves(const Board& board, Position pos) const;

        /**
         * @brief kind getter
         */
        PieceKind kind() const {
            return kind_;
        }

        /**
         * @brief colour getter
         */
        Colour colour() const {
            return colour_;
        }

    private:
        /**
         * @brief Private piece kind
         */
        PieceKind kind_;
        /**
         * @brief Private Colour
         */

        Colour colour_;

        /**
         * @brief A pointer to MoveRule (The Strategy)
         */
        std::shared_ptr<const MoveRule> rule_;
};


#endif // PIECE_HPP

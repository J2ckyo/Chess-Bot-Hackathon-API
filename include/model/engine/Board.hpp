/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

 #ifndef BOARD_HPP
 #define BOARD_HPP

 #include <vector>
 #include "Types.hpp"
 #include "Piece.hpp"

class Board{
    public:
    /**
     * @brief Board constructor to 8x8
     */
    Board()

    /**
     * @brief Checks if piece in bounds
     */
    bool in_bounds(Position pos);

    /**
     * @brief Get's the position of a piece
     */
    Piece piece_at(Position pos);

    /**
     * @brief Places piece at a position
     */
    void place(Position pos, const Piece& piece);

    private:
        /**
         * @brief 2D Vector for the board
         */
        std::vector<std::vector<int>> board;
};

 #endif // BOARD_HPP
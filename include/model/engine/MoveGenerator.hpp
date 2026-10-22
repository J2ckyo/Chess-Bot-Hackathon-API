#ifndef MOVEGENERATOR_HPP
#define MOVEGENERATOR_HPP

/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#include "Board.hpp"
#include "Square.hpp"
#include "Piece.hpp"
#include "Move.hpp"
#include <utility>
#include <vector>
#include <optional>

 /**
  * @class MoveGenerator
  * @brief Checks Move legality and ending the game
  */
class MoveGenerator {
    public:

        /**
         * @brief Can I piece reach square
         * @return bool
         */
        static bool is_attacked(const Board& board, const Square& square, Piece::Colour by);

        /**
         * @brief Is the Kind in check
         * @return bool
         */
        static bool in_check(const Board& board, Piece::Colour colour);

        /**
         * @brief Every move based on how pieces move
         * @return std::vector<Move>
         */
        static std::vector<Move> pseudo_legal(const Board& board);

        /**
         * @brief Checks legal moves
         * @return std::vector<Move>
         */
        static std::vector<Move> legal_moves(const Board& board);

        /**
         * @brief is move in legal moves, server uses
         * it to check the bots move.
         * @return bool
         */
        static bool is_legal(const Board& board, const Move& move);

    private:
        /**
         * @brief Helper function finds the king.
         */
        static std::optional<Square> find_king(const Board& board, Piece::Colour colour);

        /**
         * @brief Jump move helper function
         */
        static void jump_moves(const Board& board, const Square& from, Piece::Colour colour, const std::vector<std::pair<int, int>>& offest, std::vector<Move>& moves);

        /**
         * @brief Slide move helper function
         */
        static void slide_moves(const Board& board, const Square& from, Piece::Colour colour, const std::vector<std::pair<int, int>>& directions, std::vector<Move>& moves);

        /**
         * @brief Pawn moves helper
         */
        static void pawn_moves(const Board& board, const Square& from, Piece::Colour colour, std::vector<Move>& moves);

        /**
         * @brief Castling Moves Helper
         */
        static void castling_moves(const Board& board, Piece::Colour colour, std::vector<Move>& moves);
};

#endif

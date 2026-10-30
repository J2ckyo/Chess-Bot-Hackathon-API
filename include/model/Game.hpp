#ifndef GAME_HPP
#define GAME_HPP

/**
 * @author Jack Simonton jack.simonton@uleth.ca
 * @date Fall 2026
 */

#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include "MoveGenerator.hpp"
#include "Move.hpp"
#include "Board.hpp"

/**
 * @class Game
 */
class Game {
    public:
        /**
         * @brief Status Enum
         */
        enum Status {
            WHITE_WINS,
            BLACK_WINS,
            DRAW,
            IN_PROGRESS
        };

        /**
         * @brief Game Constructor
         */
        Game(const std::string& fen = Board::START_FEN);

        /**
         * @brief Tries a move
         * @return bool
         */
        bool try_move(const std::string& uci);

        /**
         * @brief 
         */
        Status status() const;

        /**
         * @brief 
         */
        std::string to_fen() const;

        /**
         * @brief
         */
        std::vector<std::string> history_uci() const;

        Piece::Colour side_to_move() const {return board.get_side_to_move();}

    private:
        /**
         * @brief
         */
        Board board;

        /**
         * @brief 
         */
        std::vector<Move> history;

        /**
         * @brief
         */
        Status current_status = IN_PROGRESS;

        /**
         * @brief
         */
        std::map<std::string, int> positions;

        /**
         * @brief
         */
        std::string positions_key() const;

        /**
         * @brief
         */

        void update_status();

};

#endif


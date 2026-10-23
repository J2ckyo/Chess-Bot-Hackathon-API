/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include <string>
#include <vector>

/**
 * @class GameState
 * @brief Keeps track of the GameState JSON member fields
 */

class GameState {
    public:
        /**
         * @brief Did the request succeed
         */
        bool ok = false;

        /**
         * @brief Error enum of what went wrong
         */
        enum Error {
            NONE,
            BAD_KEY,
            GAME_NOT_FOUND,
            NOT_A_PLAYER,
            NOT_YOUR_TURN,
            ILLEGAL_MOVE,
            GAME_OVER
        };

        /**
         * @brief 
         */
        Error error = NONE;

        /**
         * @brief Short error message
         */
        std::string message;

        /**
         * @brief Game ID
         */
        std::string game_id;

        /**
         * @brief fen code
         */
        std::string fen;

        /**
         * @brief Bots Colour
         */
        std::string your_colour;

        /**
         * @brief Games Status
         */
        std::string status;

        /**
         * @brief History of moves
         */
        std::vector<std::string> moves;

};

#endif

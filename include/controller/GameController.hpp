/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#ifndef GAMECONTROLLER_HPP
#define GAMECONTROLLER_HPP

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include "Game.hpp"
#include "GameState.hpp"
#include "BotRegistry.hpp"
#include "Leaderboard.hpp"

/**
 * @class Gamecontroller
 */
class GameController {
    public:
        /**
         * @brief Construct a new Game Controller object
         * 
         * @param registry
         * @param leaderboard 
         */
        GameController(BotRegistry& registry, Leaderboard& leaderboard) : registry(registry), leaderboard(leaderboard) {};

        /**
         * @brief Starts a game
         * 
         * @param ids 
         * @return std::optional<std::string> 
         */
        std::optional<std::string> start_game(const std::string& white_id, const std::string& black_id);

        /**
         * @brief Get the state object
         * 
         * @param game_id 
         * @param api_key 
         * @return std::optional<GameState> 
         */
        GameState get_state(const std::string& game_id, const std::string& api_key);

        /**
         * @brief Makes a move
         * 
         * @param game_id 
         * @param api_key 
         * @param uci 
         * @return std::optional<GameState> 
         */
        GameState make_move(const std::string& game_id, const std::string& api_key, const std::string& uci);


        private:
            struct Match {
                Game game;
                std::string white_id;
                std::string black_id;
                std::mutex lock;
            };
            
            /**
             * @brief Reference stored by constructor
             * 
             */
            BotRegistry& registry;

            /**
             * @brief Reference stored by constructor
             * 
             */
            Leaderboard& leaderboard;

            /**
             * @brief 
             * 
             */
            std::map<std::string, std::unique_ptr<Match>> games;

            /**
             * @brief 
             * 
             */
            int counter = 1;

            /**
             * @brief 
             * 
             */
            std::mutex games_lock;

            /**
             * @brief records result on leaderboard
             * 
             * @param match 
             */
            void finish(Match& match);

            /**
             * @brief Fills in a reply
             * 
             * @param match 
             * @param game_id 
             * @param bot 
             * @return GameState 
             */
            GameState build_state(const Match& match, const std::string& game_id, const Bot& bot) const;

            /**
             * @brief Locks map and find game.
             * Returns a ptr to it 
             * 
             * @param game_id 
             * @return Match* 
             */
            Match* find_match(const std::string& game_id);

            /**
             * @brief Private helper that returns GameStre with ok = false
             * 
             * @param error 
             * @param message 
             * @return GameState 
             */
            static GameState make_error(GameState::Error error, const std::string& message);

};

#endif

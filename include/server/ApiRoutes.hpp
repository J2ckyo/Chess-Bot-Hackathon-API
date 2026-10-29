/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#ifndef APIROUTES_HPP
#define APIROUTES_HPP

#include <cmath>
#include "crow.h"
#include "GameController.hpp"
#include "BotRegistry.hpp"
#include "Leaderboard.hpp"

class ApiRoutes {
    public:

        /**
         * @brief 
         * 
         * @param app 
         */
        void register_routes(crow::SimpleApp& app);

               /**
         * @brief Turns gamestate into json with the matching HTTP status code
         * 
         * @param gamecontroller 
         * @param registry 
         * @param leaderboard 
         */
        ApiRoutes(GameController& gamecontroller, BotRegistry& registry, Leaderboard& leaderboard, const std::string& admin_key) :
            gamecontroller(gamecontroller), registry(registry), leaderboard(leaderboard), admin_key(admin_key) {};

    private:
    
        GameController& gamecontroller;
        BotRegistry& registry;
        Leaderboard& leaderboard;
        std::string admin_key;

        /**
         * @brief Adds every API endpoint to app
         * 
         * @param gamestate 
         * @return crow::response 
         */
        crow::response to_response(const GameState& gamestate) const;

        /**
         * @brief Returns if header xadminkey is == admin_key
         * 
         * @param req 
         * @return true 
         * @return false 
         */
        bool is_admin(const crow::request& req) const;
};


#endif

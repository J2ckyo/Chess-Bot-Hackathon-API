/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#ifndef LEADERBOARD_HPP
#define LEADERBOARD_HPP

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <mutex>

/**
 * @class Leaderboard
 * @brief Keeps track of the bots wins and loses
 */

class Leaderboard {
    public:
        struct Rating
        {
            /* data */
            std::string bot_id = "";
            double rating = 1200;
            int wins = 0;
            int losses= 0;
            int draws = 0;
        };
        
         /**
         * @brief Status Enum seperate from game
         */
        enum Status {
            WHITE_WINS,
            BLACK_WINS,
            DRAW,
        };

        /**
         * @brief Records match results
         * 
         */
        void record(std::string& white_id, std::string& black_id, Status result);
        
        /**
         * @brief Organizes Records
         * 
         */
        std::vector<Rating> top(const int n);

        /**
         * @brief Bot lookup
         * 
         */
        std::optional<Rating> get(const std::string& bot_id) const;

    private:
        /**
         * @brief Guards map
         */
        mutable std::mutex lock;

         /**
         * @brief Every registered bot Rating
         */
        std::map<std::string, Rating> ratings;

};

#endif
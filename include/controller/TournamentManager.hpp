/**
 * @file TournamentManager.hpp
 * @author Jack Simonton jack.simonton@Uleth.ca
 * @brief 
 * @version 0.1
 * @date 2026-10-28
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef TOURNAMENTMANAGER_HPP
#define TOURNAMENTMANAGER_HPP

#include <vector>
#include <string>
#include <optional>
#include <mutex>
#include <algorithm>
#include "GameController.hpp"
#include "BotRegistry.hpp"

/**
 * @brief Tournament Class
 * 
 */
class TournamentManager {
    public: 
        struct Pairing
        {
            /* data */
            std::string white_id;
            std::string black_id;
            std::optional<std::string> game_id;
        };
        

        /**
         * @brief Construct a new Tournamentmanager object
         * 
         * @param gamecontroller 
         * @param registry 
         */
        TournamentManager(GameController& gamecontroller, BotRegistry& registry)
         : gamecontroller(gamecontroller), registry(registry){};

        /**
         * @brief Create a schedule object
         * 
         * @return true 
         * @return false 
         */
        bool create_schedule();

        /**
         * @brief Starts every game in current round
         * 
         * @return true 
         * @return false 
         */
        bool start_round();

        /**
         * @brief True when every game in round is over
         * 
         * @return true 
         * @return false 
         */
        bool round_finished();

        /**
         * @brief If current round is done, moves to next
         * 
         * @return true 
         * @return false 
         */
        bool next_round();

        /**
         * @brief A copy of current round
         * 
         * @return std::vector<Pairing> 
         */
        std::vector<Pairing> current_round() const;

        /**
         * @brief Display current round
         * 
         * @return int 
         */
        int round_number() const;

        /**
         * @brief Display total rounds
         * 
         * @return int 
         */
        int total_rounds() const;


    private:
        GameController& gamecontroller;
        BotRegistry& registry;

        /**
         * @brief Index of current round
         * 
         */
        int current = 0;

        /**
         * @brief List of each pairings
         * 
         */
        std::vector<std::vector<Pairing>> rounds;
        /**
        * @brief 
        * 
        */
        mutable std::mutex lock;

        /**
         * @brief Private help locker
         * 
         * @return true 
         * @return false 
         */
        bool round_finished_unlocked();

        /**
         * @brief Private help locker
         * 
         * @return true 
         * @return false 
         */
        bool start_round_unlocked();

};





#endif
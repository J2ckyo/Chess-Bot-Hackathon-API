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

#include <mutex>
#include "GameController.hpp"
#include "BotRegistry.hpp"

/**
 * @brief Tournament Class
 * 
 */
class Tournamentmanager {
    public: 

        void create_schedule();

        void start_round();

        bool round_finished();

        void next_round();


    private:
        GameController& gamecontroller;
        BotRegistry& registry;

        int current_round = 1;

        /**
        * @brief 
        * 
        */
        std::mutex lock;

};





#endif
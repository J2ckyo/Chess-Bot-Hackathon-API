/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#ifndef BOTREGISTRY_HPP
#define BOTREGISTRY_HPP

#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include "Bot.hpp"

/**
 * @class BotRegistry
 * @brief Keeps track of all registered bots and checks APIs
 */

class BotRegistry {
    public:
        /**
         * @brief Returns the API key & creates new bot
         */
        std::optional<std::string> register_bot(const std::string& name);

        /**
         * @brief Returns bot that owns API key
         */
        std::optional<Bot> authenticate(const std::string& key) const;

        /**
         * @brief Returns this bot with id
         */
        std::optional<Bot> get(const std::string& id) const;

        /**
         * @brief Returns a list of all bots
         */
        std::vector<Bot> all() const;

    private:
        /**
         * @brief Every registered bot
         */
        std::map<std::string, Bot> bots_by_id;

        /**
         * @brief Maps hash API key to bots ID
         */
        std::map<std::string, std::string> id_by_key_hash;

        /**
         * @brief Number for bots ID
         */
        int counter = 1;

        /**
         * @brief Guards both maps and counter
         */
        mutable std::mutex lock;

        /**
         * @brief Makes a new API key in 32 hex
         */
        std::string generate_key();

        /**
         * @brief Scrambles a key
         */
        std::string hash_key(const std::string& key) const;

};

#endif
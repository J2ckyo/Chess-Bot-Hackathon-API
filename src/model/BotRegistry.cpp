/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#include "BotRegistry.hpp"

std::optional<std::string> BotRegistry::register_bot(const std::string& name) {
    std::lock_guard<std::mutex> guard(lock);
    if (name.empty() || name.length() > 32) {
        return std::nullopt;
    }
    for (const auto& [id, bot] : bots_by_id) {
        if (name == bot.name){
            return std::nullopt;
        }
    }
    std::string bot_id = "bot-" + std::to_string(counter);
    counter += 1;

    std::string key = generate_key();
    std::string hash = hash_key(key);
    bots_by_id[bot_id] = Bot{bot_id, name, hash};
    id_by_key_hash[hash] = bot_id;
    return key;
}

std::optional<Bot> BotRegistry::authenticate(const std::string& key) const{
    std::lock_guard<std::mutex> guard(lock);
    std::string hash = hash_key(key);
    auto it = id_by_key_hash.find(hash);
    if (it != id_by_key_hash.end()) {
        return bots_by_id.find(it->second)->second;
    }
    else {
        return std::nullopt;
    }
}


std::optional<Bot> BotRegistry::get(const std::string& id) const {
    std::lock_guard<std::mutex> guard(lock);
    if (bots_by_id.find(id) == bots_by_id.end()) {
        return std::nullopt;
    }
    return bots_by_id.find(id)->second;
}

std::vector<Bot> BotRegistry::all() const {
    std::lock_guard<std::mutex> guard(lock);
    std::vector<Bot> the_bot;
    for (const auto& [id, bot] : bots_by_id) {
        the_bot.push_back(bot);
    }
    return the_bot;
}

std::string BotRegistry::generate_key() {
    std::string char_list = "0123456789abcdef";
    std::string key;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    for (int i = 0; i < 32; i++) {
        key += char_list[dis(gen)];
    }
    return key;
}

std::string BotRegistry::hash_key(const std::string& key) const {
        std::size_t hash_value =  std::hash<std::string>{} (key);
        return std::to_string(hash_value);
}




/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#include "Leaderboard.hpp"

void Leaderboard::record(std::string& white_id, std::string& black_id, Status result) {
    std::lock_guard<std::mutex> guard(lock);
}

std::vector<Leaderboard::Rating> Leaderboard::top(const int n) {
    std::lock_guard<std::mutex> guard(lock);
    std::vector<Rating> sorted_leaderboard;
    for () {
        
    }
}

std::optional<Leaderboard::Rating> Leaderboard::get(const std::string& bot_id) const {
    std::lock_guard<std::mutex> guard(lock);
    if (ratings.find(bot_id) == ratings.end()) {
        return std::nullopt;
    }
    return ratings.find(bot_id)->second;
}

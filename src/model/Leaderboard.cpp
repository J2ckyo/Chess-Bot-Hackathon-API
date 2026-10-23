/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#include "Leaderboard.hpp"

void Leaderboard::record(const std::string& white_id, const std::string& black_id, Status result) {
    std::lock_guard<std::mutex> guard(lock);
    Rating& white = ratings[white_id];
    Rating& black = ratings[black_id];
    white.bot_id = white_id;
    black.bot_id = black_id;
    double white_score;

    if (result == Status::WHITE_WINS) {
        white.wins += 1;
        black.losses += 1;
        white_score = 1;
    }
    else if (result == Status::BLACK_WINS) {
        white.losses += 1;
        black.wins += 1;
        white_score = 0.0;
    }
    else {
        white.draws += 1;
        black.draws += 1;
        white_score = 0.5;
    }

    double black_score = 1.0 - white_score;
    
    double whites_exp = 1.0 / (1.0 + std::pow(10, (black.rating - white.rating) / 400.0));
    double blacks_exp = 1.0 - whites_exp;

    white.rating += 32 * (white_score - whites_exp);
    black.rating += 32 * (black_score - blacks_exp);
}

std::vector<Leaderboard::Rating> Leaderboard::top(const int n) const {
    std::lock_guard<std::mutex> guard(lock);
    std::vector<Rating> sorted_leaderboard;
    for (const auto& [id, rating] : ratings) {
        sorted_leaderboard.push_back(rating);
    }
    std::sort(sorted_leaderboard.begin(), sorted_leaderboard.end(), [](const Rating& a, const Rating& b) {
        return a.rating > b.rating;
    });
    if (sorted_leaderboard.size() > static_cast<std::size_t>(n)) {
        sorted_leaderboard.resize(n);
    }
    return sorted_leaderboard;
}

std::optional<Leaderboard::Rating> Leaderboard::get(const std::string& bot_id) const {
    std::lock_guard<std::mutex> guard(lock);
    if (ratings.find(bot_id) == ratings.end()) {
        return std::nullopt;
    }
    return ratings.find(bot_id)->second;
}

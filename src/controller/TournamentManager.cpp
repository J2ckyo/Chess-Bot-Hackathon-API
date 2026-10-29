#include "TournamentManager.hpp"


bool TournamentManager::create_schedule() {
    std::lock_guard<std::mutex> guard(lock);
    rounds.clear();
    current = 0;
    std::vector<std::string> ids;
    for (auto& bot : registry.all()) {
        ids.push_back(bot.id);
    }

    if (ids.size() < 2) {
        return false;
    }

    if (ids.size() % 2 == 1) {
        ids.push_back("");
    }

    int n = ids.size();
    std::string white;
    std::string black;
    for (int r = 0; r < n - 1; r++) {
        std::vector<Pairing> round;
        for (int i = 0; i < n / 2; i++) {
            std::string a = ids[i];
            std::string b = ids[n - 1 - i];
            if (a == "" || b == "") {
                continue;
            }
            if (r % 2 == 0) {
                white = a;
                black = b;
            }
            else {
               white = b;
               black = a;
            }
            round.push_back(Pairing{white, black});
        }
        rounds.push_back(round);
        std::rotate(ids.begin() + 1, ids.end() - 1, ids.end());
    }
    return true;
}

bool TournamentManager::start_round() {
    std::lock_guard<std::mutex> guard(lock);
    return start_round_unlocked();
}

bool TournamentManager::round_finished() {
    std::lock_guard<std::mutex> guard(lock);
    return round_finished_unlocked();
}

bool TournamentManager::next_round() {
    std::lock_guard<std::mutex> guard(lock);
    if (round_finished_unlocked() == false) {
        return false;
    }
    current += 1;
    if (current >= rounds.size()) {
        return false;
    }
    return start_round_unlocked();
}

std::vector<TournamentManager::Pairing> TournamentManager::current_round() const {
    std::lock_guard<std::mutex> guard(lock);
    if (current < rounds.size()) {
        return rounds[current];
    }

    return {};
}

int TournamentManager::round_number() const {
    std::lock_guard<std::mutex> guard(lock);
    return current + 1;
}

int TournamentManager::total_rounds() const {
    std::lock_guard<std::mutex> guard(lock);
    return static_cast<int>(rounds.size());
}

// Private helpers

bool TournamentManager::round_finished_unlocked() {
    if (current >= rounds.size()) {
        return false;
    }

    for (auto& pairing : rounds[current]) {
        if (!pairing.game_id) {
            return false;
        }
        std::optional<std::string> game_id = pairing.game_id;
        if (gamecontroller.game_status(*game_id) == Game::IN_PROGRESS) {
            return false;
        }
    }
    return true;
}


bool TournamentManager::start_round_unlocked() {
    if (current >= rounds.size()) {
        return false;
    }

    for (auto& pairing : rounds[current]) {
        if (pairing.game_id) {
            continue;
        }
        std::string white_id = pairing.white_id;
        std::string black_id = pairing.black_id;
        std::optional<std::string> id = gamecontroller.start_game(white_id, black_id);
        if (id) {
            pairing.game_id = id;
        }
    }
    return true;
}

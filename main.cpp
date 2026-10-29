#include <string>
#include <iostream>
#include <cstdlib>
#include "ApiRoutes.hpp"
#include "BotRegistry.hpp"
#include "Leaderboard.hpp"
#include "GameController.hpp"
#include "TournamentManager.hpp"

using namespace std;

int main() {
    const char* key = std::getenv("ADMIN_KEY");
    if (key == nullptr) {
        cout << "ADMIN KEY not set";
        return 1;
    }

    std::string admin_key = key;
    
    BotRegistry registry;
    Leaderboard leaderboard;
    GameController gamecontroller(registry, leaderboard);
    TournamentManager manager(gamecontroller, registry);
    ApiRoutes routes(gamecontroller, registry, leaderboard, manager, admin_key);

    crow::SimpleApp app;
    routes.register_routes(app);
    app.port(18080).multithreaded().run();

    return 0;
};
 

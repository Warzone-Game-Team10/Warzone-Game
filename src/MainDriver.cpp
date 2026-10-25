#include "MapDriver.h"
#include "CardsDriver.h"
#include "PlayerDriver.h"
#include "OrdersDriver.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>

namespace {

void printUsage(std::ostream& out, const char* program) {
    out << "Usage: " << program << " [map|player|orders|cards|all] [maps-directory]\n"
        << "       " << program << " --help\n"
        << "Defaults: all tests, maps/ directory\n"
        << "Available components: map, player, orders, cards\n";
}

}

int main(int argc, char* argv[]) {
    const std::string_view command = argc > 1 ? argv[1] : "all";

    if (argc == 2 && command == "--help") {
        printUsage(std::cout, argv[0]);
        return 0;
    }

    if (argc > 3 ||
        (command != "map" && command != "player" &&
         command != "orders" && command != "cards" && command != "all") ||
        (argc == 3 && command == "cards")) {
        printUsage(std::cerr, argv[0]);
        return 1;
    }

    try {
        const std::filesystem::path mapDirectory =
            argc == 3 ? argv[2] : "maps";

        std::cout << "============================ COMP 345 - Warzone tests ============================\n"
                  << "Running: " << command << "\n\n";

        bool passed = true;

        // Part 1: Map
        if (command == "map" || command == "all") {
            std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= Part 1: Map -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
            if (!testMaps(mapDirectory)) {
                passed = false;
            }
        }

        // Part 2: Player
        if (command == "player" || command == "all") {
            std::cout << "\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= Part 2: Player -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";

            if (!testPlayer(mapDirectory / "nyc" / "NYC.map")) {
                passed = false;
            }
        }

        // Part 3: Orders
        if (command == "orders" || command == "all") {
            std::cout << "\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- Part 3: Orders -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";

            if (!testOrders()) {
                passed = false;
            }
        }

        // Part 4: Cards
        if (command == "cards" || command == "all") {
            std::cout << "\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- Part 4: Cards =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n";

            if (!testCards()) {
                passed = false;
            }
        }

        // Part 5: Game Engine


        

        std::cout << (passed ? "\nALL SELECTED TESTS PASSED\n"
                             : "\nSELECTED TESTS FAILED\n");

        return passed ? 0 : 1;

    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}

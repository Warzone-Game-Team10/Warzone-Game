#include "MapDriver.h"
#include "CardsDriver.h"
#include "PlayerDriver.h"
#include "OrdersDriver.h"
#include "GameEngineDriver.h"
#include "CommandProcessingDriver.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>

namespace {

void printUsage(std::ostream& out, const char* program) {
    out << "Usage: " << program << " -console\n"
        << "       " << program << " -file <filename>\n"
        << "       " << program << " [map|player|orders|cards|all] [maps-directory]\n"
        << "       " << program << " [gameengine|commands]\n"
        << "       " << program << " --help\n"
        << "Defaults: all tests, maps/ directory\n"
        << "Available components: map, player, orders, cards, gameengine, commands\n"
        << "commands: run command-processing tests.\n"
        << "all: run all tests.\n"
        << "-console: type game commands in the terminal.\n"
        << "-file <filename>: read game commands from a text file.\n"
        << "Use one option per run. Only -console and gameengine wait for keyboard input.\n";
}

int usageError(const char* program, const char* message) {
    std::cerr << "Error: " << message << '\n';
    printUsage(std::cerr, program);
    return 1;
}

}

int main(int argc, char* argv[]) {
    const std::string_view command = argc > 1 ? argv[1] : "all";

    if (argc == 2 && command == "--help") {
        printUsage(std::cout, argv[0]);
        return 0;
    }

    try {
        if (command == "-console") {
            if (argc != 2) {
                return usageError(argv[0], "-console accepts no additional arguments or input modes.");
            }
            demonstrateConsoleCommands();
            return 0;
        }
        if (command == "-file") {
            if (argc != 3 || std::string_view(argv[2]).empty() || std::string_view(argv[2]).starts_with("-")) {
                return usageError(argv[0], "-file requires one filename and no additional input mode.");
            }
            demonstrateFileCommands(argv[2]);
            return 0;
        }

        const bool acceptsMapDirectory =
            command == "map" || command == "player" || command == "orders" || command == "all";
        const bool knownComponent =
            acceptsMapDirectory || command == "cards" || command == "gameengine" || command == "commands";
        if (!knownComponent || argc > 3 ||
            (argc == 3 && (!acceptsMapDirectory || std::string_view(argv[2]).starts_with("-")))) {
            return usageError(argv[0], "Invalid driver or launch arguments.");
        }

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
        // doesnt run make run-engine cause it requirse user input
        if (command == "gameengine") {
            std::cout << "\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= Part 5: Game Engine =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";

            testGameStates();
        }

        if (command == "commands" || command == "all") {
            passed = testCommandProcessing() && passed;
        }

        std::cout << (passed ? "\nALL SELECTED TESTS PASSED\n"
                             : "\nSELECTED TESTS FAILED\n");

        return passed ? 0 : 1;

    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}

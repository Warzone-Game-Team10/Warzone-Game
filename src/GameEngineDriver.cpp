#include "GameEngine.h"
#include "GameEngineDriver.h"
#include "CommandProcessing.h"
#include "Map.h"
#include "Player.h"
#include "Cards.h"

#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>

// reads, validates, & applies cmds until input ends or the engine reaches end state
void runCommandTransitions(GameEngine& engine, CommandProcessor& processor, std::ostream& out) {
    out << engine << '\n';
    while (!engine.isFinished()) {
        out << "\n[" << engine.getCurrentStateName() << "] Enter command: " << std::flush;
        Command* command = processor.getCommand();
        if (!command) {
            out << "End of input. Command processing stopped.\n";
            return;
        }
        if (processor.validate(command, engine.getCurrentStateName())) {
            engine.processCommand(*command);
        }
        out << '\n' << *command << '\n';
    }
    out << "Quit accepted. Command processing stopped.\n";
}

// demonstrates game state transitions using console commands
void testGameStates() {
    GameEngine engine;
    CommandProcessor processor;
    runCommandTransitions(engine, processor, std::cout);
}

// test with scripted startup scenario and verifies cmd outcomes and resulting game setup
bool testStartupPhase() {
    std::cout << "===== GAME STARTUP DRIVER =====\n"
              << "Commands: loadmap, validatemap, six addplayer commands, gamestart\n";

    std::istringstream input(
        "loadmap nyc/NYC.map\n"
        "validatemap\n"
        "addplayer Noah\n"
        "addplayer Elliot\n"
        "addplayer Yveline\n"
        "addplayer Mila\n"
        "addplayer Elena\n"
        "addplayer Melody\n"
        "addplayer Helios\n"
        "gamestart\n");

    GameEngine engine;
    CommandProcessor processor(input);
    engine.setCommandProcessor(&processor);
    engine.startupPhase();

    bool passed = true;
    const auto expect = [&passed](bool condition, const std::string& message) {
        std::cout << (condition ? "PASS: " : "FAIL: ") << message << '\n';
        passed = passed && condition;
    };

    const std::vector<std::string> expectedCommands = {
        "loadmap", "validatemap", "addplayer", "addplayer", "addplayer",
        "addplayer", "addplayer", "addplayer", "addplayer", "gamestart"
    };
    expect(processor.getCommandCount() == expectedCommands.size(),
           "all startup commands, including the rejected seventh player, were read");
    const std::size_t commandCount =
        std::min(processor.getCommandCount(), expectedCommands.size());
    for (std::size_t index = 0; index < commandCount; ++index) {
        const Command* command = processor.getSavedCommand(index);
        const bool seventhPlayer = index == 8;
        const bool expectedOutcome = seventhPlayer
            ? command->getEffect().find("maximum of 6 players") != std::string::npos
            : command->getEffect().find("Transition:") == 0;
        expect(command->getName() == expectedCommands[index] && expectedOutcome,
               seventhPlayer ? "seventh player was rejected"
                             : "executed " + expectedCommands[index] + " command");
    }

    const Map* map = engine.getGameMap();
    expect(map != nullptr && map->validate(), "map was loaded and validated");
    expect(engine.getCurrentStateName() == "assign reinforcement",
           "game switched to the play phase");
    expect(engine.getPlayers().size() == 6, "six players were accepted");
    if (map == nullptr || engine.getPlayers().size() != 6) {
        std::cout << (passed ? "STARTUP DRIVER PASSED\n" : "STARTUP DRIVER FAILED\n");
        return passed;
    }

    const auto& players = engine.getPlayers();
    std::vector<std::size_t> territoryCounts;
    territoryCounts.reserve(players.size());
    for (Player* player : players) {
        territoryCounts.push_back(player->getTerritoryList().size());
        expect(player->getReinforcementPool() == 50,
               player->getName() + " received 50 reinforcement armies");
        expect(player->getCardHand() != nullptr && player->getCardHand()->getSize() == 2,
               player->getName() + " drew two initial cards");
    }

    const auto [minimum, maximum] = std::minmax_element(
        territoryCounts.begin(), territoryCounts.end());
    expect(*maximum - *minimum <= 1, "territories were dealt fairly (counts differ by at most one)");
    expect(std::accumulate(territoryCounts.begin(), territoryCounts.end(), std::size_t{0}) ==
               map->getTerritories().size(),
           "every map territory was assigned");

    const bool ownershipIsConsistent = std::all_of(
        map->getTerritories().begin(), map->getTerritories().end(),
        [&players](const auto& territory) {
            const auto owner = std::find(players.begin(), players.end(), territory->getOwner());
            return owner != players.end() && (*owner)->ownsTerritory(territory.get());
        });
    expect(ownershipIsConsistent, "every territory has a matching player owner");

    std::cout << "\nRandomized play order and startup allocations:\n";
    for (std::size_t index = 0; index < players.size(); ++index) {
        std::cout << index + 1 << ". " << players[index]->getName()
                  << " - territories: " << territoryCounts[index]
                  << ", reinforcement pool: " << players[index]->getReinforcementPool()
                  << ", hand: " << *players[index]->getCardHand() << '\n';
    }

    std::istringstream minimumInput(
        "gamestart\n"
        "loadmap tests/invalid_maps/disconnected_map.map\n"
        "validatemap\n"
        "loadmap nyc/NYC.map\n"
        "validatemap\n"
        "addplayer Solo\n"
        "gamestart\n"
        "addplayer Duo\n"
        "gamestart\n");
    GameEngine minimumEngine;
    CommandProcessor minimumProcessor(minimumInput);
    minimumEngine.setCommandProcessor(&minimumProcessor);
    minimumEngine.startupPhase();
    const bool invalidStateCommandWasRejected =
        minimumProcessor.getCommandCount() >= 2 &&
        minimumProcessor.getSavedCommand(0)->getName() == "gamestart" &&
        minimumProcessor.getSavedCommand(0)->getEffect().find(
            "not allowed in state 'start'") != std::string::npos &&
        minimumProcessor.getSavedCommand(1)->getEffect().find(
            "'start' -> 'map loaded'") != std::string::npos;
    expect(invalidStateCommandWasRejected,
           "gamestart is rejected in start and the engine remains able to load a map");
    const bool invalidMapWasRejected =
        minimumProcessor.getCommandCount() >= 3 &&
        minimumProcessor.getSavedCommand(2)->getEffect().find("map is invalid") !=
            std::string::npos;
    expect(invalidMapWasRejected, "disconnected map was rejected and could be replaced");
    const bool minimumWasEnforced =
        minimumProcessor.getCommandCount() == 9 &&
        minimumProcessor.getSavedCommand(6)->getEffect().find("at least 2 players") !=
            std::string::npos &&
        minimumEngine.getPlayers().size() == 2 &&
        minimumEngine.getCurrentStateName() == "assign reinforcement";
    expect(minimumWasEnforced, "one player cannot start; two players can");

    std::cout << (passed ? "\nSTARTUP DRIVER PASSED\n" : "\nSTARTUP DRIVER FAILED\n");
    return passed;
}
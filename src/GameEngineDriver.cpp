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


    // true if every territory of the copied map is owned by one of the copy's own players
    // (never by a player of the original) and has the same owner name as in the original
    // assumes the map copy keeps the territory order
    const auto ownersPointToCopy = [](const GameEngine& copy, const GameEngine& original) {
        const Map* copiedMap = copy.getGameMap();
        const Map* originalMap = original.getGameMap();
        if (copiedMap == nullptr || originalMap == nullptr ||
            copiedMap->getTerritories().size() != originalMap->getTerritories().size()) {
            return false;
        }
        const auto& copiedPlayers = copy.getPlayers();
        const auto& originalPlayers = original.getPlayers();
        for (std::size_t index = 0; index < copiedMap->getTerritories().size(); ++index) {
            Player* copiedOwner = copiedMap->getTerritories()[index]->getOwner();
            Player* originalOwner = originalMap->getTerritories()[index]->getOwner();
            const bool ownedByCopy = std::find(copiedPlayers.begin(), copiedPlayers.end(),
                                               copiedOwner) != copiedPlayers.end();
            const bool ownedByOriginal = std::find(originalPlayers.begin(), originalPlayers.end(),
                                                   copiedOwner) != originalPlayers.end();
            if (!ownedByCopy || ownedByOriginal ||
                copiedOwner->getName() != originalOwner->getName()) {
                return false;
            }
        }
        return true;
    };

    GameEngine copiedEngine(engine);
    expect(copiedEngine.getPlayers().size() == players.size() &&
               copiedEngine.getPlayers()[0] != players[0],
           "copy constructor: the copy has its own players");
    expect(ownersPointToCopy(copiedEngine, engine),
           "copy constructor: copied territories are owned by the copied players");

    // assigning over an engine that already owns players and a map also tests freeing the old data
    minimumEngine = engine;
    expect(minimumEngine.getPlayers().size() == players.size() &&
               ownersPointToCopy(minimumEngine, engine),
           "assignment operator: copied territories are owned by the copied players");

    // true if every player's hand points at the given engine's own deck
    const auto handsUseEngineDeck = [](const GameEngine& owner) {
        for (Player* player : owner.getPlayers()) {
            if (player->getCardHand()->getDeck() != owner.getDeck()) {
                return false;
            }
        }
        return true;
    };

    // true if drawing from and returning a card to each hand's deck works (writes through the pointer)
    const auto deckIsUsableThroughHands = [](const GameEngine& owner) {
        for (Player* player : owner.getPlayers()) {
            Deck* handDeck = player->getCardHand()->getDeck();
            const int sizeBefore = handDeck->getSize();
            Card* card = handDeck->draw();
            if (card == nullptr) {
                return false;
            }
            handDeck->returnCard(card);
            if (handDeck->getSize() != sizeBefore) {
                return false;
            }
        }
        return true;
    };

    // the source engine is destroyed while its copies are still alive
    GameEngine* source = new GameEngine(engine);
    GameEngine survivorByCopy(*source);
    GameEngine survivorByAssignment(engine);   // already owns players and a deck, so the old ones are freed too
    survivorByAssignment = *source;
    const bool copiesHaveOwnDecks =
        survivorByCopy.getDeck() != source->getDeck() &&
        survivorByAssignment.getDeck() != source->getDeck() &&
        handsUseEngineDeck(survivorByCopy) && handsUseEngineDeck(survivorByAssignment);
    delete source;

    expect(copiesHaveOwnDecks, "copies: every copied hand points at the copy's own deck");
    expect(deckIsUsableThroughHands(survivorByCopy),
           "copy constructor: hands still reach a live deck after the original engine is destroyed");
    expect(deckIsUsableThroughHands(survivorByAssignment),
           "assignment operator: hands still reach a live deck after the original engine is destroyed");

    std::cout << (passed ? "\nSTARTUP DRIVER PASSED\n" : "\nSTARTUP DRIVER FAILED\n");
    return passed;
}
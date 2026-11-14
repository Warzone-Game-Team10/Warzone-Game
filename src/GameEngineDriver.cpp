#include "GameEngine.h"
#include "GameEngineDriver.h"
#include "CommandProcessing.h"
#include "Map.h"
#include "Player.h"
#include "Cards.h"
#include "CardOrderIntegration.h"

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





// =====================================================================
// PART 3 DRIVER: Main Game Loop & Gameplay Phases
// =====================================================================
// Part 3 driver: each required behavior has an explicit expected result.
bool testMainGameLoop() {
    bool passed = true;
    const auto expect = [&passed](bool condition, const std::string& message) {
        std::cout << (condition ? "PASS: " : "FAIL: ") << message << '\n';
        passed = passed && condition;
    };
    // A connected 12-territory map, with two continents of six territories each.
    Map map;
    Continent* west = map.addContinent("West", 5);
    Continent* east = map.addContinent("East", 2);
    for (int i = 0; i < 12; ++i) map.addTerritory(i + 1, "T" + std::to_string(i + 1), i < 6 ? west : east);
    for (const auto& a : map.getTerritories())
        for (const auto& b : map.getTerritories()) if (a != b) a->addAdjacentTerritory(b.get());

    std::cout << "\n=== Part 3: reinforcement cases ===\n";
    GameEngine reinforcements;
    reinforcements.setupGame(map, {"Alice", "Bob"});
    Player* alice = reinforcements.getPlayers()[0];
    Player* bob = reinforcements.getPlayers()[1];
    reinforcements.reinforcementPhase();
    expect(alice->getReinforcementPool() == 3 && bob->getReinforcementPool() == 3,
           "six territories without a complete continent: minimum three armies");
    for (int id : {2, 4, 6}) expect(bob->transferTerritoryTo(*alice, id), "transfer western territory");
    alice->setReinforcementPool(4);
    reinforcements.reinforcementPhase();
    expect(alice->getReinforcementPool() == 12,
           "nine territories: floor(9/3) + West bonus 5 = 8, added to existing pool 4");
    for (Territory* t : bob->getTerritoryList()) bob->transferTerritoryTo(*alice, t->getId());
    alice->setReinforcementPool(0);
    bob->setReinforcementPool(0);
    reinforcements.reinforcementPhase();
    expect(alice->getReinforcementPool() == 11, "twelve territories plus both continent bonuses: 4+5+2=11");
    expect(bob->getReinforcementPool() == 0, "territoryless player receives no reinforcement");

    std::cout << "\n=== Part 3: player decisions and cards ===\n";
    GameEngine decisions;
    decisions.setupGame(map, {"Alice", "Bob"});
    alice = decisions.getPlayers()[0]; bob = decisions.getPlayers()[1];
    alice->setReinforcementPool(6);
    alice->beginIssuing();
    expect(alice->issueOrder(), "player issues a deploy decision first");
    expect(dynamic_cast<Deploy*>(alice->getOrdersList()->getOrder(0)) != nullptr,
           "first order is Deploy while armies remain uncommitted");
    expect(alice->getReinforcementPool() == 6, "issuing reserves armies; execution will consume the pool");
    expect(alice->issueOrder() && alice->issueOrder(), "player issues defensive and attacking advances");
    expect(dynamic_cast<Advance*>(alice->getOrdersList()->getOrder(1)) &&
           dynamic_cast<Advance*>(alice->getOrdersList()->getOrder(2)),
           "both decisions create Advance orders using defend/attack lists");
    // Execute just the first two so movement is deterministic and does not involve combat randomness.
    Territory* first = alice->toDefend().front();
    Territory* second = alice->toDefend()[1];
    alice->getOrdersList()->getOrder(0)->execute(); alice->getOrdersList()->remove(0);
    alice->getOrdersList()->getOrder(0)->execute(); alice->getOrdersList()->remove(0);
    expect(first->getArmyCount() == 5 && second->getArmyCount() == 1 && alice->getReinforcementPool() == 0,
           "deploy and defensive advance change actual map armies correctly");
    alice->getOrdersList()->remove(0); // Attack execution is demonstrated below against a zero-army target.

    for (CardType type : {CardType::BOMB, CardType::BLOCKADE, CardType::AIRLIFT, CardType::DIPLOMACY}) {
        GameEngine cards;
        cards.setupGame(map, {"CardPlayer", "Opponent"});
        Player* player = cards.getPlayers()[0];
        Card* card = new Card(type);
        player->getCardHand()->addCard(card);
        int deckSize = cards.getDeck()->getSize();
        player->beginIssuing();
        while (player->issueOrder()) {}
        bool found = false;
        OrdersList* list = player->getOrdersList();
        for (int i = 0; i < list->getSize(); ++i) {
            Order* order = list->getOrder(i);
            found = found || (type == CardType::BOMB && dynamic_cast<Bomb*>(order)) ||
                (type == CardType::BLOCKADE && dynamic_cast<Blockade*>(order)) ||
                (type == CardType::AIRLIFT && dynamic_cast<Airlift*>(order)) ||
                (type == CardType::DIPLOMACY && dynamic_cast<Negotiate*>(order));
        }
        expect(found && player->getCardHand()->getSize() == 0 && cards.getDeck()->getSize() == deckSize + 1,
               "card type " + std::to_string(static_cast<int>(type)) + " queues its order and returns card to deck");
        expect(!player->issueOrder(), "a passed player remains finished for this turn");
    }

    std::cout << "\n=== Part 3: execution priority and round robin ===\n";
    GameEngine execution;
    execution.setupGame(map, {"Alice", "Bob"});
    alice = execution.getPlayers()[0]; bob = execution.getPlayers()[1];
    alice->setReinforcementPool(4); bob->setReinforcementPool(4);
    Territory* a = alice->toDefend()[0]; Territory* a2 = alice->toDefend()[1];
    Territory* b = bob->toDefend()[0]; Territory* b2 = bob->toDefend()[1];
    // Put an Advance at the front deliberately: every Deploy must still execute first.
    alice->getOrdersList()->addOrder(new Advance(alice, a, a2, 1));
    bob->getOrdersList()->addOrder(new Advance(bob, b, b2, 1));
    alice->getOrdersList()->addOrder(new Deploy(alice, a, 2));
    bob->getOrdersList()->addOrder(new Deploy(bob, b, 2));
    alice->getOrdersList()->addOrder(new Deploy(alice, a, 2));
    bob->getOrdersList()->addOrder(new Deploy(bob, b, 2));
    std::ostringstream executionLog;
    auto* old = std::cout.rdbuf(executionLog.rdbuf());
    execution.executeOrdersPhase();
    std::cout.rdbuf(old);
    std::cout << executionLog.str();
    std::vector<std::string> executionNames;
    std::istringstream lines(executionLog.str()); std::string line;
    while (std::getline(lines, line)) {
        if (line.starts_with("Executing Alice:")) executionNames.push_back("Alice");
        if (line.starts_with("Executing Bob:")) executionNames.push_back("Bob");
    }
    expect(executionNames == std::vector<std::string>({"Alice", "Bob", "Alice", "Bob", "Alice", "Bob"}),
           "execution takes one order per player per round");
    expect(a->getArmyCount() == 3 && a2->getArmyCount() == 1 &&
           b->getArmyCount() == 3 && b2->getArmyCount() == 1,
           "all deploys execute before advances, even when advances were queued first");
    expect(execution.getCurrentStateName() == "assign reinforcement", "execution returns to reinforcement state");
    expect(alice->getOrdersList()->getSize() == 0 && bob->getOrdersList()->getSize() == 0, "all orders were removed");
    execution.issueOrdersPhase();
    expect(alice->getOrdersList()->getSize() > 0 && bob->getOrdersList()->getSize() > 0,
           "round-robin issuing runs to player passes without a fixed round cap");

    std::cout << "\n=== Part 3: conquest, elimination, victory and replay ===\n";
    Map duel;
    Continent* continent = duel.addContinent("Duel", 1);
    Territory* left = duel.addTerritory(1, "Left", continent);
    Territory* right = duel.addTerritory(2, "Right", continent);
    left->addAdjacentTerritory(right); right->addAdjacentTerritory(left);
    GameEngine game;
    game.setupGame(duel, {"Winner", "Loser"});
    Player* winner = game.getPlayers()[0]; Player* loser = game.getPlayers()[1];
    // With zero defenders, the attack succeeds without randomness.
    winner->getOrdersList()->addOrder(new Advance(winner, winner->toDefend()[0], loser->toDefend()[0], 1));
    winner->setReinforcementPool(1);
    winner->getOrdersList()->addOrder(new Deploy(winner, winner->toDefend()[0], 1));
    game.executeOrdersPhase();
    expect(game.getPlayers().size() == 1 && winner->getTerritoryList().size() == 2,
           "successful attack removes the player that lost its final territory");
    expect(game.getCurrentStateName() == "win", "complete map ownership enters win during execution");
    expect(winner->getCardHand()->getSize() == 1, "conqueror receives one card for the turn");
    expect(game.processCommand("replay") && game.getCurrentStateName() == "start" &&
           game.getPlayers().empty() && game.getGameMap() == nullptr, "replay clears old game and returns to start");
    game.setupGame(duel, {"A", "B"});
    // Complete main loop with the regular combat rules.
    game.getPlayers()[0]->setReinforcementPool(50);
    game.mainGameLoop();
    expect(game.getCurrentStateName() == "win", "mainGameLoop runs the three phases and stops on victory");
    expect(game.processCommand("quit") && game.isFinished(), "quit from win ends the application");

    std::cout << "\n=== Part 3: neutral territory prevents premature victory ===\n";
    GameEngine neutralGame;
    neutralGame.setupGame(duel, {"Blockader", "Survivor"});
    Player* blockader = neutralGame.getPlayers()[0];
    Player* remaining = neutralGame.getPlayers()[1];
    Territory* neutral = blockader->toDefend()[0];
    neutral->setArmyCount(4);
    blockader->getOrdersList()->addOrder(new Blockade(blockader, neutral));
    neutralGame.executeOrdersPhase();
    expect(neutral->getOwner() == nullptr && neutral->getArmyCount() == 8,
           "blockade preserves the map territory and doubles its armies under neutral ownership");
    expect(neutralGame.getPlayers().size() == 1 && neutralGame.getCurrentStateName() != "win",
           "one surviving player cannot win while a map territory remains neutral");
    neutral->setArmyCount(0);
    remaining->toDefend()[0]->setArmyCount(1);
    remaining->getOrdersList()->addOrder(new Advance(remaining, remaining->toDefend()[0], neutral, 1));
    neutralGame.executeOrdersPhase();
    expect(neutralGame.getCurrentStateName() == "win" && neutral->getOwner() == remaining,
           "capturing the final neutral territory triggers victory");

    GameEngine deferred;
    deferred.setupGame(duel, {"Issuer", "Other"});
    Player* issuer = deferred.getPlayers()[0];
    Card* bomb = new Card(CardType::BOMB);
    issuer->getCardHand()->addCard(bomb);
    Territory* ownTarget = issuer->toDefend()[0];
    ownTarget->setArmyCount(6);
    expect(playCardWithOrder(bomb, issuer, new Bomb(issuer, ownTarget)),
           "a matching card can queue an order that is illegal in the current map state");
    deferred.executeOrdersPhase();
    expect(ownTarget->getArmyCount() == 6 && issuer->getOrdersList()->getSize() == 0,
           "invalid order is rejected only at execution and leaves armies unchanged");

    std::cout << "\n=== Part 3: map identity and copy lifetime ===\n";
    GameEngine survivor;
    {
        GameEngine original;
        original.setupGame(map, {"A", "B"});
        original.reinforcementPhase();
        original.issueOrdersPhase();
        survivor = original;
        GameEngine constructed(original);
        expect(constructed.getPlayers()[0]->toDefend()[0] == constructed.getGameMap()->getTerritories()[0].get(),
               "engine copy constructor binds players to its own map");
    }
    survivor.executeOrdersPhase();
    bool canonicalOwnership = true;
    for (Player* player : survivor.getPlayers()) for (Territory* owned : player->getTerritoryList()) {
        bool matchesMap = false;
        for (const auto& territory : survivor.getGameMap()->getTerritories())
            if (territory.get() == owned && territory->getOwner() == player) matchesMap = true;
        canonicalOwnership = canonicalOwnership && matchesMap;
    }
    expect(canonicalOwnership, "assigned engine executes copied orders after original engine destruction");
    std::cout << (passed ? "PART 3 DRIVER PASSED\n" : "PART 3 DRIVER FAILED\n");
    return passed;
}

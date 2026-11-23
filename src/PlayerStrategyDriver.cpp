#include "Cards.h"
#include "Orders.h"
#include <sstream>
#include <vector>
#include "PlayerStrategyDriver.h"
#include "PlayerStrategies.h"
#include "Player.h"
#include "Map.h"
#include "Cards.h"
#include "Orders.h"
#include "GameEngine.h"


// ----------------------------------------------------------------------------
// HELPERS 
// ----------------------------------------------------------------------------

// Prints PASS or FAIL for one check. A failed check sets allPassed to false
// but does not stop the test, so every failure is reported in one run.
void check(bool& allPassed, bool condition, const std::string& message) {
    if (condition) {
        std::cout << "  PASS: " << message << "\n";
    } else {
        std::cout << "  FAIL: " << message << "\n";
        allPassed = false;
    }
}

// Loads the map file with the project's MapLoader (throws if the file is bad).
std::unique_ptr<Map> loadTestMap(const std::string& path) {
    MapLoader loader;
    return loader.loadMap(path);
}

// Finds a territory of the map by its id (the number used in the .map file).
Territory* findTerritoryById(const Map& map, int id) {
    const std::vector<std::unique_ptr<Territory>>& territories = map.getTerritories();
    for (size_t i = 0; i < territories.size(); i++) {
        if (territories[i]->getId() == id) {
            return territories[i].get();
        }
    }
    throw std::runtime_error("Territory id not found in map");
}

// Returns the territories with the given ids, in the given order.
std::vector<Territory*> collectTerritories(const Map& map, const std::vector<int>& ids) {
    std::vector<Territory*> result;
    for (size_t i = 0; i < ids.size(); i++) {
        result.push_back(findTerritoryById(map, ids[i]));
    }
    return result;
}

// "A, B, C" for a list of territories.
std::string joinNames(const std::vector<Territory*>& territories) {
    std::string text;
    for (size_t i = 0; i < territories.size(); i++) {
        if (i > 0) {
            text += ", ";
        }
        text += territories[i]->getName();
    }
    return text;
}

// Prints how many territories a player owns and which ones.
void printPlayerTerritories(Player& player) {
    std::cout << "  " << player.getName() << " owns "
            << player.getTerritoryList().size() << ": "
            << joinNames(player.getTerritoryList()) << "\n";
}    

// Joins script lines with newlines. Each line is one thing the user would type.
std::string joinLines(const std::vector<std::string>& lines) {
    std::string script;
    for (size_t i = 0; i < lines.size(); i++) {
        script += lines[i] + "\n";
    }
    return script;
}

// Text of one order in a player's orders list (description + effect).
std::string describeOrder(Player& player, int index) {
    std::ostringstream text;
    text << *(player.getOrdersList()->getOrder(index));
    return text.str();
}

// Runs one issuing phase for a player (call issueOrder() until it returns false)
// with std::cin replaced by the script. Returns how many orders were issued.
// When the script runs out the strategy sees end-of-input. std::cin is always restored.
int runScriptedTurn(Player& player, const std::string& script) {
    std::istringstream scriptedInput(script);
    std::streambuf* originalBuffer = std::cin.rdbuf(scriptedInput.rdbuf());

    int ordersIssued = 0;
    try {
        // The limit only protects the test from looping forever if a script is wrong.
        while (ordersIssued < 100 && player.issueOrder()) {
            ordersIssued++;
        }
    } catch (...) {
        std::cin.rdbuf(originalBuffer);
        std::cin.clear();
        throw;
    }

    std::cin.rdbuf(originalBuffer);
    std::cin.clear();
    return ordersIssued;
}

// Same idea, but runs GameEngine's real issuing phase (round-robin between all
// players). The script holds the answers in the order the engine asks for them.
void runEngineIssuingPhase(GameEngine& engine, const std::string& script) {
    std::istringstream scriptedInput(script);
    std::streambuf* originalBuffer = std::cin.rdbuf(scriptedInput.rdbuf());

    try {
        engine.issueOrdersPhase();
    } catch (...) {
        std::cin.rdbuf(originalBuffer);
        std::cin.clear();
        throw;
    }

    std::cin.rdbuf(originalBuffer);
    std::cin.clear();
}

// Runs GameEngine::mainGameLoop() with std::cin replaced by the script.
void runEngineMainLoop(GameEngine& engine, const std::string& script) {
    std::istringstream scriptedInput(script);
    std::streambuf* originalBuffer = std::cin.rdbuf(scriptedInput.rdbuf());

    try {
        engine.mainGameLoop();
    } catch (...) {
        std::cin.rdbuf(originalBuffer);
        std::cin.clear();
        throw;
    }

    std::cin.rdbuf(originalBuffer);
    std::cin.clear();
}

// ----------------------------------------------------------------------------
// PLAYER STRATEGY CLONE TEST
// Returns true only if every check passed.
// ----------------------------------------------------------------------------

bool testPlayerStrategyClone() {
    std::cout << "\n===== PLAYER STRATEGY CLONE TEST =====\n";
    bool allPassed = true;
    Player source;
    Player assigned;
    CheaterPlayerStrategy sourceStrategy(&source);
    sourceStrategy.finishedIssuing = true;

    std::unique_ptr<PlayerStrategy> copiedStrategy(sourceStrategy.clone());
    check(allPassed,copiedStrategy.get() != &sourceStrategy &&
                      copiedStrategy->player == &source &&
                      copiedStrategy->finishedIssuing,
                      "clone creates an independent strategy with copied state");
    copiedStrategy->finishedIssuing = false;
    check(allPassed,sourceStrategy.finishedIssuing,
                      "changing copied state leaves the source unchanged");
    copiedStrategy->setPlayer(&assigned);
    check(allPassed,copiedStrategy->player == &assigned &&
                      sourceStrategy.player == &source,
                      "rebinding a copy leaves the source strategy unchanged");

    CheaterPlayerStrategy assignedStrategy(&assigned);
    assignedStrategy.finishedIssuing = false;
    assignedStrategy = sourceStrategy;
    check(allPassed,assignedStrategy.player == &assigned &&
                      assignedStrategy.finishedIssuing,
                      "strategy assignment copies state without changing its owner");

    return allPassed;
}

// ----------------------------------------------------------------------------
// HUMAN STRATEGY TEST
// Returns true only if every check passed.
// ----------------------------------------------------------------------------
//
// Layout on NYC.map:
//   Alice (human) owns 33 North-Island, 34 Mid-Island, 35 South-Island,
//                      1 Financial-District, 2 Midtown (in this order)
//   Bob owns 15 Williamsburg, 26 Park-Slope, 32 Bay-Ridge
//   Carol owns 3 Central-Park
// Alice's enemy neighbours: [Bay-Ridge, Williamsburg, Park-Slope, Central-Park]
bool testHumanPlayerStrategy(const std::string& mapPath) {
    std::cout << "\n===== HUMAN STRATEGY TEST (scripted input) =====\n";
    bool allPassed = true;

    try {
        // The map and deck must outlive the players, so they are declared first.
        std::unique_ptr<Map> map = loadTestMap(mapPath);
        Deck deck;

        Player alice("Alice", nullptr, nullptr, std::vector<Territory*>());
        Player bob("Bob", nullptr, nullptr, std::vector<Territory*>());
        Player carol("Carol", nullptr, nullptr, std::vector<Territory*>());

        alice.bindMapTerritories(collectTerritories(*map, {33, 34, 35, 1, 2}));
        bob.bindMapTerritories(collectTerritories(*map, {15, 26, 32}));
        carol.bindMapTerritories(collectTerritories(*map, {3}));

        findTerritoryById(*map, 33)->setArmyCount(4);
        findTerritoryById(*map, 34)->setArmyCount(2);
        findTerritoryById(*map, 35)->setArmyCount(3);
        findTerritoryById(*map, 1)->setArmyCount(5);
        findTerritoryById(*map, 2)->setArmyCount(1);

        alice.addReinforcementArmies(6);

        // A fixed hand, so the test is not random.
        alice.getCardHand()->setDeck(&deck);
        alice.getCardHand()->addCard(new Card(CardType::BOMB));
        alice.getCardHand()->addCard(new Card(CardType::DIPLOMACY));

        std::cout << "\n-- Setup --\n";
        check(allPassed,
              dynamic_cast<HumanPlayerStrategy*>(alice.getStrategy()) != nullptr &&
              alice.getStrategy()->player == &alice,
              "a new Player has a Human strategy bound to itself");
        check(allPassed,
              alice.toDefend().size() == 5 &&
              joinNames(alice.toAttack()) == "Bay-Ridge, Williamsburg, Park-Slope, Central-Park",
              "Player::toDefend() / toAttack() delegate to the strategy");

        // ---- Turn 1 ----
        std::cout << "\n-- Turn 1 (scripted input) --\n";
        alice.beginIssuing();
        std::vector<std::string> turnOne = {
            "abc",          // deploy territory: not a number -> rejected, asks again
            "4",            // deploy to #4 Financial-District
            "4",            // 4 armies                          (Deploy order)
            "1", "2",       // deploy 2 armies to #1 North-Island (Deploy order)
            "1", "4", "2",  // menu: advance, source #4 Financial-District, target #2 Williamsburg
            "6",            // 6 armies                          (Advance order, an attack)
            "2", "0",       // menu: play a card, then 0 = cancel -> back to the menu
            "2", "1", "3",  // menu: card #1 Bomb, target #3 Park-Slope   (Bomb order)
            "2", "1", "2",  // menu: card #1 is now Diplomacy, opponent #2 Carol (Negotiate order)
            "3"             // menu: end turn
        };
        int issued = runScriptedTurn(alice, joinLines(turnOne));

        OrdersList* orders = alice.getOrdersList();
        check(allPassed, issued == 5 && orders->getSize() == 5,
              "5 orders were issued from the script");
        check(allPassed,
              dynamic_cast<Deploy*>(orders->getOrder(0)) != nullptr &&
              dynamic_cast<Deploy*>(orders->getOrder(1)) != nullptr &&
              dynamic_cast<Advance*>(orders->getOrder(2)) != nullptr &&
              dynamic_cast<Bomb*>(orders->getOrder(3)) != nullptr &&
              dynamic_cast<Negotiate*>(orders->getOrder(4)) != nullptr,
              "order types: Deploy, Deploy, Advance, Bomb, Negotiate");
        check(allPassed,
              describeOrder(alice, 0).find("Deploy 4 armies to Financial-District") != std::string::npos &&
              describeOrder(alice, 2).find("Advance 6 armies from Financial-District to Williamsburg") != std::string::npos &&
              describeOrder(alice, 4).find("Negotiate with Carol") != std::string::npos,
              "the orders contain the territories, armies and player that were chosen");
        check(allPassed, alice.getCardHand()->getSize() == 0 && deck.getSize() == 17,
              "both played cards left the hand and went back to the deck");
        check(allPassed, alice.issueOrder() == false,
              "after ending the turn, issueOrder() returns false");

        // The orders are real: executing the Deploy orders changes the map.
        orders->getOrder(0)->execute();
        orders->getOrder(1)->execute();
        check(allPassed,
              findTerritoryById(*map, 1)->getArmyCount() == 9 &&
              findTerritoryById(*map, 33)->getArmyCount() == 6,
              "executing the Deploy orders adds the armies (5 -> 9 and 4 -> 6)");


                // ---- Execute the remaining orders: Advance, Bomb, Negotiate (in list order) ----
        std::cout << "\n-- Executing the other orders --\n";
        std::cout << "Before: Financial-District has "
                  << findTerritoryById(*map, 1)->getArmyCount()
                  << " armies, Williamsburg belongs to "
                  << findTerritoryById(*map, 15)->getOwner()->getName() << "\n";

        orders->getOrder(2)->execute();   // Advance 6: Financial-District -> Williamsburg
        orders->getOrder(3)->execute();   // Bomb Park-Slope
        orders->getOrder(4)->execute();   // Negotiate with Carol

        std::cout << "After: Financial-District has "
                  << findTerritoryById(*map, 1)->getArmyCount()
                  << " armies, Williamsburg belongs to "
                  << findTerritoryById(*map, 15)->getOwner()->getName() << " with "
                  << findTerritoryById(*map, 15)->getArmyCount() << " armies\n";

        check(allPassed,
              findTerritoryById(*map, 15)->getOwner() == &alice &&
              findTerritoryById(*map, 15)->getArmyCount() == 6 &&
              findTerritoryById(*map, 1)->getArmyCount() == 3,
              "executing the Advance: Williamsburg (0 defenders) is conquered with 6 armies, "
              "Financial-District drops from 9 to 3");
        check(allPassed,
              alice.getTerritoryList().size() == 6 && bob.getTerritoryList().size() == 2,
              "ownership moved from Bob to Alice: 6 and 2 territories");
        check(allPassed, alice.isNegotiatingWith(carol),
              "executing the Negotiate: Alice and Carol are now negotiating");
        
              
        // ---- Turn 2: beginIssuing() must reset the strategy's per-turn state ----
        std::cout << "\n-- Turn 2 --\n";
        alice.addReinforcementArmies(3);
        alice.beginIssuing();
        issued = runScriptedTurn(alice, joinLines({"2", "3", "3"}));   // deploy 3 to #2, end turn
        check(allPassed,
              issued == 1 && orders->getSize() == 6 &&
              describeOrder(alice, 5).find("Deploy 3 armies to Mid-Island") != std::string::npos,
              "a new turn starts with a fresh deploy (per-turn state was reset)");

        // ---- Edge cases ----
        std::cout << "\n-- Edge cases --\n";
        Player eve("Eve", nullptr, nullptr, std::vector<Territory*>());
        eve.bindMapTerritories(collectTerritories(*map, {20}));
        eve.addReinforcementArmies(2);
        eve.beginIssuing();
        issued = runScriptedTurn(eve, "");   // the input ends immediately
        check(allPassed, issued == 0 && eve.getOrdersList()->getSize() == 0,
              "input ending mid-turn ends the turn instead of hanging");

        Player ivan("Ivan", nullptr, nullptr, std::vector<Territory*>());
        ivan.setStrategy(nullptr);
        check(allPassed,
              ivan.getStrategy() == nullptr && ivan.issueOrder() == false &&
              ivan.toAttack().empty() && ivan.toDefend().empty(),
              "a Player with no strategy returns false / empty lists instead of crashing");

        // ---- Two humans playing through the real GameEngine ----
        // setupGame() deals territories round-robin WITHOUT shuffling, so every run is
        // identical: Alice gets the odd ids (18 territories), Bob the even ids (17).
        std::cout << "\n-- Two humans inside a real GameEngine --\n";
        std::unique_ptr<Map> engineMap = loadTestMap(mapPath);
        GameEngine engine;
        engine.setupGame(*engineMap, std::vector<std::string>{"Alice", "Bob"});
        Player* first = engine.getPlayers()[0];
        Player* second = engine.getPlayers()[1];

        engine.reinforcementPhase();
        check(allPassed,
              first->getReinforcementPool() == 6 && second->getReinforcementPool() == 5,
              "the engine gave the reinforcements: max(3, territories / 3) = 6 and 5");

        // The engine asks Alice for one order, then Bob for one order, and repeats.
        runEngineIssuingPhase(engine, joinLines({
            "1", "6",              // round 1, Alice: deploy all 6 to #1 Financial-District
            "1", "5",              // round 1, Bob:   deploy all 5 to #1 Midtown
            "1", "1", "3", "5",    // round 2, Alice: advance, source #1, target #3 Park-Slope (Bob), 5 armies
            "3",                   // round 2, Bob:   end turn
            "3"                    // round 3, Alice: end turn
        }));
        check(allPassed,
              first->getOrdersList()->getSize() == 2 && second->getOrdersList()->getSize() == 1,
              "the engine collected 2 orders from Alice and 1 from Bob");

        engine.executeOrdersPhase();
        Territory* parkSlope = findTerritoryById(*engine.getGameMap(), 26);
        check(allPassed,
              parkSlope->getOwner() == first && parkSlope->getArmyCount() == 5 &&
              first->getTerritoryList().size() == 19 && second->getTerritoryList().size() == 16,
              "executing the orders: Alice (0 defenders) conquered Park-Slope");
        check(allPassed,
              first->getCardHand()->getSize() == 1 && second->getCardHand()->getSize() == 0,
              "only the player who conquered received a card");

    } catch (const std::exception& error) {
        std::cout << "  FAIL: unexpected exception: " << error.what() << "\n";
        allPassed = false;
    }

    if (allPassed) {
        std::cout << "\nPASS: Human strategy test\n";
    } else {
        std::cout << "\nFAIL: Human strategy test\n";
    }
    return allPassed;
}


// ----------------------------------------------------------------------------
// CHEATER STRATEGY TEST
// Returns true only if every check passed.
// ----------------------------------------------------------------------------
//
// Layout on NYC.map:
//   Cheater owns 33 North-Island, 34 Mid-Island
//   Victim owns 1 Financial-District, 2 Midtown, 15 Williamsburg, 26 Park-Slope
//   Dana owns 32 Bay-Ridge;  everything else is unowned
// Turn 1 targets: 1 (Victim's) and 35 (unowned). Turn 2 targets: 2, 15, 26, 32.
bool testCheaterPlayerStrategy(const std::string& mapPath) {
    std::cout << "\n===== CHEATER STRATEGY TEST =====\n";
    bool allPassed = true;

    try {
        std::unique_ptr<Map> map = loadTestMap(mapPath);
        Deck deck;

        Player cheater("Cheater", nullptr, nullptr, std::vector<Territory*>());
        Player victim("Victim", nullptr, nullptr, std::vector<Territory*>());
        Player dana("Dana", nullptr, nullptr, std::vector<Territory*>());

        cheater.bindMapTerritories(collectTerritories(*map, {33, 34}));
        victim.bindMapTerritories(collectTerritories(*map, {1, 2, 15, 26}));
        dana.bindMapTerritories(collectTerritories(*map, {32}));

        CheaterPlayerStrategy cheaterStrategy(&cheater);
        cheater.setStrategy(&cheaterStrategy);

        std::cout << "\n-- Setup --\n";
        check(allPassed,
              dynamic_cast<CheaterPlayerStrategy*>(cheater.getStrategy()) != nullptr &&
              cheater.getStrategy() != &cheaterStrategy &&
              cheater.getStrategy()->player == &cheater,
              "setStrategy() gives the player its own Cheater strategy copy, bound to it");
        check(allPassed,
              joinNames(cheater.toAttack()) == "Financial-District, South-Island",
              "toAttack() lists only the adjacent territories");

        // ---- Turn 1 ----
        std::cout << "\n-- Turn 1 --\n";
        cheater.beginIssuing();
        int issued = runScriptedTurn(cheater, "");   // a cheater never reads input
        check(allPassed,
              issued == 1 &&
              findTerritoryById(*map, 1)->getOwner() == &cheater &&
              findTerritoryById(*map, 35)->getOwner() == &cheater,
              "conquered both an enemy-owned and an unowned neighbour");
        check(allPassed,
              findTerritoryById(*map, 2)->getOwner() == &victim &&
              findTerritoryById(*map, 32)->getOwner() == &dana,
              "no chain reaction: territories 2 steps away were not taken");
        check(allPassed, cheater.issueOrder() == false && cheater.getTerritoryList().size() == 4,
              "a second call in the same turn does nothing (once per turn)");
        check(allPassed, cheater.getOrdersList()->getSize() == 0,
              "the cheater conquers directly: no orders are created");
        check(allPassed,
              cheater.hasConqueredThisTurn() && cheater.awardConquestCard(deck) &&
              cheater.getCardHand()->getSize() == 1,
              "the conquest earns a card");
        cheater.resetTurnState();

        // ---- Turn 2 ----
        std::cout << "\n-- Turn 2 --\n";
        cheater.beginIssuing();
        issued = runScriptedTurn(cheater, "");
        check(allPassed,
              issued == 1 && cheater.getTerritoryList().size() == 8 &&
              victim.getTerritoryList().empty() && dana.getTerritoryList().empty(),
              "beginIssuing() re-arms the cheater: it takes the next ring from two players");
        cheater.resetTurnState();

        // ---- Switching the strategy while the game runs ----
        std::cout << "\n-- Dynamic strategy change --\n";
        HumanPlayerStrategy humanStrategy(&cheater);
        cheater.setStrategy(&humanStrategy);
        size_t territoriesBefore = cheater.getTerritoryList().size();
        cheater.beginIssuing();
        issued = runScriptedTurn(cheater, "3\n");   // a human with nothing to deploy: end turn
        check(allPassed,
              dynamic_cast<HumanPlayerStrategy*>(cheater.getStrategy()) != nullptr &&
              issued == 0 && cheater.getTerritoryList().size() == territoriesBefore,
              "switched to Human: the player asks for input and conquers nothing");

        cheater.setStrategy(&cheaterStrategy);
        cheater.beginIssuing();
        issued = runScriptedTurn(cheater, "");
        check(allPassed,
              dynamic_cast<CheaterPlayerStrategy*>(cheater.getStrategy()) != nullptr &&
              issued == 1 && cheater.getTerritoryList().size() > territoriesBefore,
              "switched back to Cheater: it conquers again");
        cheater.resetTurnState();

        // ---- Copy and assignment keep a separate strategy ----
        std::cout << "\n-- Deep copy --\n";
        Player copy(cheater);
        check(allPassed,
              copy.getStrategy() != cheater.getStrategy() &&
              dynamic_cast<CheaterPlayerStrategy*>(copy.getStrategy()) != nullptr &&
              copy.getStrategy()->player == &copy,
              "copy constructor: the strategy is cloned and bound to the copy");

        Player assigned("Assigned", nullptr, nullptr, std::vector<Territory*>());
        assigned = cheater;
        check(allPassed,
              dynamic_cast<CheaterPlayerStrategy*>(assigned.getStrategy()) != nullptr &&
              assigned.getStrategy() != cheater.getStrategy() &&
              assigned.getStrategy()->player == &assigned,
              "assignment operator: the strategy is cloned and bound to the assigned player");

        // ---- Two cheaters playing a whole game through the real GameEngine ----
        // Alice (odd ids) plays first and on NYC.map every even territory touches an
        // odd one, so her first cheating turn takes the whole map: the engine must
        // then declare her the winner.
        std::cout << "\n-- Cheater vs Cheater inside a real GameEngine --\n";
        std::unique_ptr<Map> engineMap = loadTestMap(mapPath);
        GameEngine engine;
        engine.setupGame(*engineMap, std::vector<std::string>{"Alice", "Bob"});
        Player* alicePlayer = engine.getPlayers()[0];
        Player* bobPlayer = engine.getPlayers()[1];

        CheaterPlayerStrategy prototype(nullptr);
        alicePlayer->setStrategy(&prototype);
        bobPlayer->setStrategy(&prototype);

        engine.mainGameLoop();   // reinforcement / issue / execute until someone wins

        bool aliceOwnsEverything = true;
        const std::vector<std::unique_ptr<Territory>>& allTerritories = engine.getGameMap()->getTerritories();
        for (size_t i = 0; i < allTerritories.size(); i++) {
            if (allTerritories[i]->getOwner() != alicePlayer) {
                aliceOwnsEverything = false;
            }
        }
        check(allPassed,
              engine.getCurrentStateName() == "win" &&
              engine.getPlayers().size() == 1 && engine.getPlayers()[0] == alicePlayer,
              "the engine reached the win state with only Alice left");
        check(allPassed,
              aliceOwnsEverything && bobPlayer->getTerritoryList().empty(),
              "Alice owns all the territories and Bob owns none");


        // ---- A scripted human against a cheater, through the real GameEngine ----
        // The engine asks the human for one order, then the cheater takes every
        // territory next to its own. On NYC.map that is all 18 of the human's.
        std::cout << "\n-- Human (scripted) vs Cheater inside a real GameEngine --\n";
        std::unique_ptr<Map> duelMap = loadTestMap(mapPath);
        GameEngine duel;
        duel.setupGame(*duelMap, std::vector<std::string>{"Human", "Cheater"});
        Player* humanPlayer = duel.getPlayers()[0];
        Player* cheaterPlayer = duel.getPlayers()[1];

        CheaterPlayerStrategy duelPrototype(nullptr);
        cheaterPlayer->setStrategy(&duelPrototype);   // the human keeps the default Human strategy

        std::cout << "Before the game:\n";
        printPlayerTerritories(*humanPlayer);
        printPlayerTerritories(*cheaterPlayer);

        runEngineMainLoop(duel, joinLines({
            "1", "6",   // the human's only order: deploy all 6 armies to #1
            "3"         // spare answer (end turn), used only if the human still had territories
        }));

        std::cout << "After the game:\n";
        printPlayerTerritories(*humanPlayer);
        printPlayerTerritories(*cheaterPlayer);

        check(allPassed,
              duel.getCurrentStateName() == "win" &&
              duel.getPlayers().size() == 1 && duel.getPlayers()[0] == cheaterPlayer,
              "the engine declared the Cheater the winner");
        check(allPassed,
              humanPlayer->getTerritoryList().empty() &&
              cheaterPlayer->getTerritoryList().size() == 35,
              "the human lost all 18 territories, the Cheater owns all 35");
        check(allPassed, cheaterPlayer->getCardHand()->getSize() == 1,
              "the Cheater earned a card for its conquest turn");


    } catch (const std::exception& error) {
        std::cout << "  FAIL: unexpected exception: " << error.what() << "\n";
        allPassed = false;
    }

    if (allPassed) {
        std::cout << "\nPASS: Cheater strategy test\n";
    } else {
        std::cout << "\nFAIL: Cheater strategy test\n";
    }
    return allPassed;
}

bool testPlayerStrategy() {
    const std::string mapPath = "maps/nyc/NYC.map";
    bool clonePassed = testPlayerStrategyClone();
    bool humanPassed = testHumanPlayerStrategy(mapPath);
    bool cheaterPassed = testCheaterPlayerStrategy(mapPath);

    return clonePassed && humanPassed && cheaterPassed;
   
}
#include <stdio.h>
#include <iostream>

#include "Player.h"
#include "Cards.h"
#include "Order.h"
#include "Map.h"

using namespace std;

// void testPlayer(){

//     //create a player object
//     Player* player1 = new Player();

//     // show player name
//     cout << "Player name: " << player1->getName() << endl;

//     // Change player names
//     player1->setName("Alice");
//     cout << "Player name after setName(): " << player1->getName() << endl;

 
//     // show toDefend()
//     player1->toDefend();
 
//     // show toAttack()
//     player1->toAttack();

//     //add orders to the player's order list
//     player1->issueOrder();


//     // free memory
//     delete player1;

// }

// int main(){

//     testPlayer();

//     return 0;
// };


// ----------------------------------------------------------------------------
// SMALL HELPERS (plain loops, no tricks)
// ----------------------------------------------------------------------------
 
// true if the list holds a territory with this id
bool listContainsId(std::vector<Territory*> list, int id) {
    int count = list.size();
    for (int i = 0; i < count; i++) {
        if (list[i]->getId() == id) {
            return true;
        }
    }
    return false;
}
 
// true if the same territory id appears twice in the list
bool listHasDuplicates(std::vector<Territory*> list) {
    int count = list.size();
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (list[i]->getId() == list[j]->getId()) {
                return true;
            }
        }
    }
    return false;
}
 
// Takes territories number 'from' up to (but not including) 'to' from the Map.
// ".get()" turns the Map's smart pointer into a plain Territory* WITHOUT taking
// ownership. The Map still owns these territories.
std::vector<Territory*> sliceOfMap(Map* gameMap, int from, int to) {
    std::vector<Territory*> result;
    for (int i = from; i < to; i++) {
        result.push_back(gameMap->getTerritories()[i].get());
    }
    return result;
}
 
// Builds a Player using the 4-parameter constructor.
// The constructor makes its OWN copies of the cards and orders, so we delete
// the temporary ones we made right after. (If your constructor instead
// ADOPTED the pointers, remove those two deletes or you would delete them twice.)
Player* makePlayer(std::string name, std::vector<Territory*> territories) {
    Cards* tempCards = new Cards();
    OrdersList* tempOrders = new OrdersList();
 
    Player* player = new Player(name, tempCards, tempOrders, territories);
 
    delete tempCards;
    delete tempOrders;
    return player;
}
 
// Prints PASS/FAIL for one check and returns the result.
bool check(std::string description, bool condition) {
    std::cout << "   [" << (condition ? "PASS" : "FAIL") << "] " << description << std::endl;
    return condition;
}
 
// Prints up to 'maxToShow' territory names from a list.
void printList(std::string title, std::vector<Territory*> list, int maxToShow) {
    int count = list.size();
    std::cout << "   " << title << " (" << count << "):";
    for (int i = 0; i < count && i < maxToShow; i++) {
        std::cout << " " << list[i]->getName();
    }
    if (count > maxToShow) {
        std::cout << " ... +" << (count - maxToShow) << " more";
    }
    std::cout << std::endl;
}
 
// ----------------------------------------------------------------------------
// INDEPENDENT ORACLE: what SHOULD toAttack() return?
// Works directly on the Map's own territories (not on the Player's copies):
// for every territory whose id is in 'owned', look at its neighbours and keep
// those whose id is NOT in 'owned' (no duplicates).
// ----------------------------------------------------------------------------
std::vector<Territory*> expectedAttackTargets(Map* gameMap, std::vector<Territory*> owned) {
    std::vector<Territory*> expected;
 
    int total = gameMap->getTerritories().size();
    for (int i = 0; i < total; i++) {
        Territory* t = gameMap->getTerritories()[i].get();
        if (listContainsId(owned, t->getId()) == false) {
            continue;   // not owned, so its neighbours do not matter
        }
 
        std::vector<Territory*> neighbours = t->getAdjacentTerritories();
        int neighbourCount = neighbours.size();
        for (int j = 0; j < neighbourCount; j++) {
            Territory* n = neighbours[j];
            bool isOwned = listContainsId(owned, n->getId());
            bool alreadyThere = listContainsId(expected, n->getId());
            if (isOwned == false && alreadyThere == false) {
                expected.push_back(n);
            }
        }
    }
    return expected;
}
 
// ----------------------------------------------------------------------------
// Runs every check for ONE player.
// 'owned' = the territories we gave this player.
// ----------------------------------------------------------------------------
bool testPlayerLists(std::string label, Player* player,
                     std::vector<Territory*> owned, Map* gameMap) {
    std::cout << "-- " << label << " --" << std::endl;
 
    std::vector<Territory*> defend = player->toDefend();
    std::vector<Territory*> attack = player->toAttack();
    std::vector<Territory*> expected = expectedAttackTargets(gameMap, owned);
 
    printList("toDefend()", defend, 8);
    printList("toAttack()", attack, 8);
 
    bool ok = true;
 
    // ---- toDefend ----
    ok = check("toDefend() has as many territories as the player owns",
               defend.size() == owned.size()) && ok;
 
    bool defendAllOwned = true;
    int ownedCount = owned.size();
    for (int i = 0; i < ownedCount; i++) {
        if (listContainsId(defend, owned[i]->getId()) == false) {
            defendAllOwned = false;
        }
    }
    ok = check("every owned territory is in toDefend()", defendAllOwned) && ok;
 
    // ---- toAttack ----
    ok = check("toAttack() has the expected number of targets",
               attack.size() == expected.size()) && ok;
 
    bool allExpectedPresent = true;
    int expectedCount = expected.size();
    for (int i = 0; i < expectedCount; i++) {
        if (listContainsId(attack, expected[i]->getId()) == false) {
            allExpectedPresent = false;
        }
    }
    ok = check("every expected neighbour is in toAttack()", allExpectedPresent) && ok;
 
    bool noneOwned = true;
    int attackCount = attack.size();
    for (int i = 0; i < attackCount; i++) {
        if (listContainsId(owned, attack[i]->getId())) {
            noneOwned = false;
        }
    }
    ok = check("toAttack() contains no territory the player owns", noneOwned) && ok;
    ok = check("toAttack() has no duplicates", listHasDuplicates(attack) == false) && ok;
 
    return ok;
}
 
// ----------------------------------------------------------------------------
// MAIN TEST FUNCTION  (call this from your main, or use the standalone main below)
// ----------------------------------------------------------------------------
bool testToDefendAndToAttack(std::string mapFilePath) {
    // ---- Load the map. loadMap() returns a unique_ptr<Map>; .release() hands
    // us the ordinary pointer, so WE must "delete gameMap" at the end.
    MapLoader* loader = new MapLoader();
    Map* gameMap = nullptr;
    try {
        gameMap = loader->loadMap(mapFilePath).release();
    } catch (std::exception& error) {
        std::cout << "Could not load map '" << mapFilePath << "': " << error.what() << std::endl;
        delete loader;
        return false;
    }
    delete loader;
 
    int total = gameMap->getTerritories().size();
    std::cout << "Loaded '" << mapFilePath << "' with " << total << " territories\n" << std::endl;
 
    bool allPassed = true;
    int half = total / 2;
 
    // ---- Test 1: two players split the map in half ----
    std::cout << "=== Test 1: two players, map split in half ===" << std::endl;
    std::vector<Territory*> aliceLand = sliceOfMap(gameMap, 0, half);
    std::vector<Territory*> bobLand = sliceOfMap(gameMap, half, total);
    Player* alice = makePlayer("Alice", aliceLand);
    Player* bob = makePlayer("Bob", bobLand);
 
    allPassed = testPlayerLists("Alice", alice, aliceLand, gameMap) && allPassed;
    allPassed = testPlayerLists("Bob", bob, bobLand, gameMap) && allPassed;
 
    // Alice owns one half and Bob owns the other, so everything Alice can
    // attack must be Bob's land (and so must appear in Bob's toDefend()).
    std::vector<Territory*> aliceTargets = alice->toAttack();
    std::vector<Territory*> bobDefend = bob->toDefend();
    bool targetsAreBobs = true;
    int targetCount = aliceTargets.size();
    for (int i = 0; i < targetCount; i++) {
        if (listContainsId(bobDefend, aliceTargets[i]->getId()) == false) {
            targetsAreBobs = false;
        }
    }
    allPassed = check("every territory Alice can attack is in Bob's toDefend()", targetsAreBobs) && allPassed;
    std::cout << std::endl;
 
    // ---- Test 2: a player with NO territories ----
    std::cout << "=== Test 2: player with no territories ===" << std::endl;
    std::vector<Territory*> nothing;
    Player* nobody = makePlayer("Nobody", nothing);
    allPassed = testPlayerLists("Nobody", nobody, nothing, gameMap) && allPassed;
    std::cout << std::endl;
 
    // ---- Test 3: a player with exactly ONE territory ----
    std::cout << "=== Test 3: player with one territory ===" << std::endl;
    std::vector<Territory*> justOne = sliceOfMap(gameMap, 0, 1);
    Player* solo = makePlayer("Solo", justOne);
    allPassed = testPlayerLists("Solo", solo, justOne, gameMap) && allPassed;
    std::cout << std::endl;
 
    // ---- Test 4: a player who owns EVERY territory (nothing left to attack) ----
    std::cout << "=== Test 4: player owns the whole map ===" << std::endl;
    std::vector<Territory*> everything = sliceOfMap(gameMap, 0, total);
    Player* ruler = makePlayer("Ruler", everything);
    allPassed = testPlayerLists("Ruler", ruler, everything, gameMap) && allPassed;
    allPassed = check("toAttack() is empty", ruler->toAttack().size() == 0) && allPassed;
    std::cout << std::endl;
 
    // ---- Cleanup: players first, THEN the map ----
    delete alice;
    delete bob;
    delete nobody;
    delete solo;
    delete ruler;
    delete gameMap;
 
    std::cout << (allPassed ? "ALL CHECKS PASSED" : "SOME CHECKS FAILED") << std::endl;
    return allPassed;
}
 
// ----------------------------------------------------------------------------
// OPTIONAL standalone main. It only exists if you compile with
//     -DPLAYERDRIVER2_STANDALONE
// so that it does not clash with the main() in your MainDriver.cpp.
// ----------------------------------------------------------------------------
#ifdef PLAYERDRIVER2_STANDALONE
int main(int argc, char* argv[]) {
    std::string mapFile = "maps/sample.map";
    if (argc > 1) {
        mapFile = argv[1];
    }
    bool passed = testToDefendAndToAttack(mapFile);
    return passed ? 0 : 1;
}
#endif
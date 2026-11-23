#include "PlayerStrategies.h"

#include "CardOrderIntegration.h"
#include "Cards.h"
#include "Map.h"
#include "Orders.h"
#include "Player.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>


namespace {
    // Asks the user for an integer between minimum and maximum (inclusive).
    // Keeps asking until the input is valid. Returns false if the input stream ended.
    bool askForNumber(const std::string& prompt, int minimum, int maximum, int& result) {
        while (true) {
            std::cout << prompt << " [" << minimum << "-" << maximum << "]: ";

            std::string line;
            if (!std::getline(std::cin, line)) {
                return false;
            }

            std::istringstream parser(line);
            int value = 0;
            std::string leftover;

            bool readNumber = static_cast<bool>(parser >> value);
            bool hasExtraText = static_cast<bool>(parser >> leftover);

            if (readNumber && !hasExtraText && value >= minimum && value <= maximum) {
                result = value;
                return true;
            }

            std::cout << "Invalid input, please try again.\n";
        }
    }

    // Prints a numbered list of territories with their armies and owner.
    void printTerritoryChoices(const std::vector<Territory*>& territories) {
        for (size_t i = 0; i < territories.size(); i++) {
            Territory* territory = territories[i];
            std::cout << "  " << (i + 1) << ") " << territory->getName()
                    << " (armies: " << territory->getArmyCount() << ", owner: ";

            Player* owner = territory->getOwner();
            if (owner != nullptr) {
                std::cout << owner->getName();
            } else {
                std::cout << "none";
            }
            std::cout << ")\n";
        }
    }


    int getValueOrZero(const std::unordered_map<int, int>& values, int key) {
        auto found = values.find(key);
        if (found == values.end()) {
            return 0;
        }
        return found->second;
    }

    
}


// ==================================================
// PlayerStrategy (abstract base class)
// ==================================================

// Creates a strategy for the given player.
PlayerStrategy::PlayerStrategy(Player* owner) {
    player = owner;
    finishedIssuing = false;
}

// Copy constructor.
PlayerStrategy::PlayerStrategy(const PlayerStrategy& other)
    : player(other.player), finishedIssuing(other.finishedIssuing) {}

// Assignment operator.
PlayerStrategy& PlayerStrategy::operator=(const PlayerStrategy& other) {
    if (this != &other) {
        // Keep this strategy bound to its current player.
        finishedIssuing = other.finishedIssuing;
    }

    return *this;
}

// Destructor. The player is borrowed, so nothing is deleted here.
PlayerStrategy::~PlayerStrategy() {
}

// Forgets everything that happened during the previous turn.
void PlayerStrategy::beginIssuing() {
    finishedIssuing = false;
}

void PlayerStrategy::setPlayer(Player* owner) {
    player = owner;
}

// helper functioin to find the territories that touch one of the player's territories but are not the player's own.
std::vector<Territory*> PlayerStrategy::findAdjacentEnemyTerritories() const {
    std::vector<Territory*> enemies;

    if (player == nullptr) {
        return enemies;
    }

    std::vector<Territory*> owned = player->getTerritoryList();

    for (size_t i = 0; i < owned.size(); i++) {
        const std::vector<Territory*>& neighbours = owned[i]->getAdjacentTerritories();

        for (size_t j = 0; j < neighbours.size(); j++) {
            Territory* candidate = neighbours[j];

            // ignore an empty neighbour slot
            if (candidate == nullptr) {
                continue;
            }

            // a territory of the player is not an enemy
            if (player->ownsTerritory(candidate)) {
                continue;
            }

            // do not list the same territory twice
            bool alreadyListed = false;
            for (size_t k = 0; k < enemies.size(); k++) {
                if (enemies[k]->getId() == candidate->getId()) {
                    alreadyListed = true;
                }
            }

            if (!alreadyListed) {
                enemies.push_back(candidate);
            }
        }
    }

    return enemies;
}

// The territories owned by the player.
std::vector<Territory*> PlayerStrategy::findOwnedTerritories() const {
    std::vector<Territory*> owned;

    if (player != nullptr) {
        owned = player->getTerritoryList();
    }

    return owned;
}

// Polymorphic stream insertion: works through a PlayerStrategy reference or pointer.
std::ostream& operator<<(std::ostream& out, const PlayerStrategy& strategy) {
    out << "Player Strategy abstract base class";
    return out;
}

// ==================================================
// HumanPlayerStrategy
// ==================================================
HumanPlayerStrategy::HumanPlayerStrategy(Player* player) : PlayerStrategy(player), committedDeployArmies(0) {
}
HumanPlayerStrategy::HumanPlayerStrategy(const HumanPlayerStrategy& other) : PlayerStrategy(other),
      committedDeployArmies(other.committedDeployArmies),
      deployedThisTurn(other.deployedThisTurn),
      advancedThisTurn(other.advancedThisTurn) {
}
PlayerStrategy* HumanPlayerStrategy::clone() const {
    return new HumanPlayerStrategy(*this);
}

HumanPlayerStrategy& HumanPlayerStrategy::operator=(const HumanPlayerStrategy& other) {
    if (this != &other) {
        PlayerStrategy::operator=(other);
        committedDeployArmies = other.committedDeployArmies;
        deployedThisTurn = other.deployedThisTurn;
        advancedThisTurn = other.advancedThisTurn;
    }
    return *this;
}

HumanPlayerStrategy::~HumanPlayerStrategy() {}

// Forget everything that happened during the previous turn.
void HumanPlayerStrategy::beginIssuing() {
    PlayerStrategy::beginIssuing();
    committedDeployArmies = 0;
    deployedThisTurn.clear();
    advancedThisTurn.clear();
}

// Armies on a territory that can still be moved: current armies, plus armies deployed
// there this turn, minus armies already ordered out of it.
//Ex: territory A has 5 armies, 3 were deployed at Territory A, and 4 have already been ordered out of Territory A. The return value is 4 (5 + 3 - 4).
int HumanPlayerStrategy::getArmiesAvailableToMove(Territory* territory) const {
    // Returns the number of armies that can still be moved from the given territory.
    int armies = territory->getArmyCount();
    // Add the armies that have been deployed there this turn.
    armies += getValueOrZero(deployedThisTurn, territory->getId());

    // Subtract the armies that have already been ordered out of this territory.
    armies -= getValueOrZero(advancedThisTurn, territory->getId());
    return armies;
}

// Issues ONE order chosen by the user. Returns false only when the user's turn is over.
bool HumanPlayerStrategy::issueOrder() {
    // If the player has already finished issuing orders, or if the player pointer is null, return false.
    if (finishedIssuing || player == nullptr) {
        return false;
    }

    // If the player has no territories or no orders list, they cannot issue any orders.
    if (player->getTerritoryList().empty() || player->getOrdersList() == nullptr) {
        finishedIssuing = true;
        return false;
    }

    // Display the player's territories and the territories they can attack, then ask for an order.
    std::cout << "\n===== " << player->getName() << " (Human): issue an order =====\n";
    std::cout << "Your territories:\n";
    printTerritoryChoices(toDefend());
    // for (size_t i = 0; i < owned.size(); i++) {
    //     std::cout << "  " << (i + 1) << ") " << owned[i]->getName()
    //             << " (armies: " << owned[i]->getArmyCount()
    //             << ", available to move: " << getArmiesAvailableToMove(owned[i]) << ")\n";
    // }
    std::cout << "Territories you could attack:\n";
    printTerritoryChoices(toAttack());

    // All reinforcements must be deployed before any other order.
    // If the player has armies left to deploy, ask for a Deploy order first.
    int armiesLeftToDeploy = player->getReinforcementPool() - committedDeployArmies;
    if (armiesLeftToDeploy > 0) {
        return issueDeployOrder(armiesLeftToDeploy);
    }

    // If the player has no armies left to deploy, show the menu of other order types.
    return issueMenuOrder();
}

// Asks the user where to deploy and how many armies.
bool HumanPlayerStrategy::issueDeployOrder(int armiesLeftToDeploy) {
    // get the territories that can be defended (owned by the player)
    std::vector<Territory*> defend = toDefend();

    // display the territories that can be defended 
    std::cout << "You still have " << armiesLeftToDeploy << " armies to deploy.\n";
    printTerritoryChoices(defend);

    // ask the user to choose a territory and how many armies to deploy there
    int territoryChoice = 0;
    if (!askForNumber("Deploy to which territory", 1, static_cast<int>(defend.size()), territoryChoice)) {
        finishedIssuing = true;
        return false;
    }

    int armyCount = 0;
    if (!askForNumber("How many armies", 1, armiesLeftToDeploy, armyCount)) {
        finishedIssuing = true;
        return false;
    }

    // create a new Deploy order and add it to the player's orders list
    Territory* target = defend[territoryChoice - 1];
    player->getOrdersList()->addOrder(new Deploy(player, target, armyCount));

    // Update the committedDeployArmies and deployedThisTurn to reflect the new deployment.
    committedDeployArmies += armyCount;
    deployedThisTurn[target->getId()] += armyCount;
    return true;
}

// Shows the menu again until an order is issued or the user ends the turn.
bool HumanPlayerStrategy::issueMenuOrder() {

    // Show the menu and ask the user to choose an order type. Keep asking until an order is issued or the user ends the turn.
    while (true) {
        // Display the menu of order types.
        std::cout << "What do you want to do?\n"
                  << "  1) Advance armies (move or attack)\n"
                  << "  2) Play a card\n"
                  << "  3) End turn\n";

        int menuChoice = 0;
        // Ask the user to choose an order type. If the input stream ends, mark the turn as finished and return false.
        if (!askForNumber("Your choice", 1, 3, menuChoice)) {
            finishedIssuing = true;
            return false;
        }

        if (menuChoice == 1) {
            // advanceOrder lets you move or attack armies
            if (issueAdvanceOrder()) {
                return true;
            }
        } else if (menuChoice == 2) {
            // playCardOrder lets you play a card from your hand such as Airlift, Bomb, Blockade, etc.
            if (issueCardOrder()) {
                return true;
            }
        } else {
            // The user chose to end their turn. Mark the turn as finished and return false.
            finishedIssuing = true;
            return false;
        }

        // The sub-menu was cancelled. Stop if the input stream ended, else show the menu again.
        if (finishedIssuing) {
            return false;
        }
    }
}

// Asks for a source, a neighbouring target and a number of armies. Entering 0 cancels.
bool HumanPlayerStrategy::issueAdvanceOrder() {
    // Get the player's territories that need to be defended 
    std::vector<Territory*> owned = toDefend();
    std::vector<Territory*> sources;

    // Only include territories from var owned that have armies available to move and add them to the sources vector
    for (size_t i = 0; i < owned.size(); i++) {
        if (getArmiesAvailableToMove(owned[i]) > 0) {
            sources.push_back(owned[i]);
        }
    }

    // If there are no territories with armies available to move, inform the user and return false.
    if (sources.empty()) {
        std::cout << "None of your territories has armies available to move.\n";
        return false;
    }

    // display territories and number of armies available to move
    std::cout << "Move armies from:\n";
    for (size_t i = 0; i < sources.size(); i++) {
        std::cout << "  " << (i + 1) << ") " << sources[i]->getName()
                  << " (available: " << getArmiesAvailableToMove(sources[i]) << ")\n";
    }

    // ask the user to choose a source territory. Entering 0 cancels the advance order.
    int sourceChoice = 0;
    if (!askForNumber("Source (0 to cancel)", 0, static_cast<int>(sources.size()), sourceChoice)) {
        finishedIssuing = true;
        return false;
    }

    // If the user entered 0, cancel the advance order and return false.
    if (sourceChoice == 0) {
        return false;
    }

    // else get the chosen source territory from the sources vector
    Territory* source = sources[sourceChoice - 1];

    // Get the neighbouring territories of the chosen source territory and add them to the targets vector
    const std::vector<Territory*>& neighbours = source->getAdjacentTerritories();
    std::vector<Territory*> targets;
    // Only include non-null neighbours in the targets vector
    for (size_t i = 0; i < neighbours.size(); i++) {
        if (neighbours[i] != nullptr) {
            targets.push_back(neighbours[i]);
        }
    }

    // If there are no neighbouring territories, inform the user and return false.
    if (targets.empty()) {
        std::cout << source->getName() << " has no neighbours.\n";
        return false;
    }

    // display the neighbouring territories and ask the user to choose a target territory. Entering 0 cancels the advance order.
    std::cout << "Move to:\n";
    printTerritoryChoices(targets);

    // ask the user to choose a target territory. Entering 0 cancels the advance order.
    int targetChoice = 0;
    if (!askForNumber("Target (0 to cancel)", 0, static_cast<int>(targets.size()), targetChoice)) {
        finishedIssuing = true;
        return false;
    }
    // If the user entered 0, cancel the advance order and return false.
    if (targetChoice == 0) {
        return false;
    }

    // else get the chosen target territory from the targets vector
    Territory* target = targets[targetChoice - 1];

    // Ask the user how many armies to move. The maximum is the number of armies available to move from the source territory.
    int available = getArmiesAvailableToMove(source);
    int armyCount = 0;
    if (!askForNumber("How many armies", 1, available, armyCount)) {
        finishedIssuing = true;
        return false;
    }

    // Create a new Advance order and add it to the player's orders list. Update the advancedThisTurn map to reflect the new advance.
    player->getOrdersList()->addOrder(new Advance(player, source, target, armyCount));

    // Inform the user that the order has been queued and will be carried out in the execute phase.
    std::cout << "Order queued: advance " << armyCount << " armies from "
          << source->getName() << " to " << target->getName()
          << " (carried out in the execute phase).\n";

    advancedThisTurn[source->getId()] += armyCount;
    return true;
}

// Lets the user pick a card from the hand and issues the matching order. these cards are: Bomb, Blockade, Airlift, and Diplomacy
bool HumanPlayerStrategy::issueCardOrder() {
    // Get the player's hand of cards. If the hand is null or the deck is null, inform the user and return false.
    Hand* hand = player->getCardHand();

    if (hand == nullptr || hand->getDeck() == nullptr) {
        std::cout << "You cannot play cards right now.\n";
        return false;
    }

    const std::vector<Card*>& cards = hand->getCards();
    if (cards.empty()) {
        std::cout << "Your hand is empty.\n";
        return false;
    }

    // Display the player's cards and ask the user to choose a card to play. Entering 0 cancels the card order.
    std::cout << "Your cards:\n";
    for (size_t i = 0; i < cards.size(); i++) {
        std::cout << "  " << (i + 1) << ") " << cards[i]->getTypeName() << "\n";
    }

    int cardChoice = 0;
    if (!askForNumber("Card (0 to cancel)", 0, static_cast<int>(cards.size()), cardChoice)) {
        finishedIssuing = true;
        return false;
    }
    if (cardChoice == 0) {
        return false;
    }

    // if user chose a card, build the corresponding order and play the card. If the order cannot be built or the card cannot be played, inform the user and return false

    // card variable is the card chosen by the user from the cards vector
    Card* card = cards[cardChoice - 1];

    
    int movedSourceId = -1; //source is the territory from which armies are moved
    int movedArmies = 0; //movedArmies is the number of armies moved from the source territory
    // 
    Order* order = buildCardOrder(card, movedSourceId, movedArmies);
    if (order == nullptr) {
        return false;
    }

    // cards::playCardWithOrder takes ownership of 'order' and deletes it if it refuses the card.
    if (!playCardWithOrder(card, player, order)) {
        std::cout << "The card could not be played.\n";
        return false;
    }

    // If armies were moved from a territory, update the advancedThisTurn map to reflect the new advance.
    if (movedArmies > 0) {
        advancedThisTurn[movedSourceId] += movedArmies;
    }
    return true;
}

// Asks the extra questions each card needs. Returns nullptr if the user cancels or the
// card cannot be used. For an Airlift, movedSourceId/movedArmies report the armies moved.
Order* HumanPlayerStrategy::buildCardOrder(Card* card, int& movedSourceId, int& movedArmies) {
    //get the territories that can be defended and attacked
    std::vector<Territory*> defend = toDefend();
    std::vector<Territory*> attack = toAttack();

    // Card BOMB: ask the user which enemy territory to bomb. If there are no enemy territories, inform the user and return nullptr.
    if (card->getType() == CardType::BOMB) {
        if (attack.empty()) {
            std::cout << "There is no enemy territory to bomb.\n";
            return nullptr;
        }
        printTerritoryChoices(attack);
        int choice = 0;
        if (!askForNumber("Bomb which territory (0 to cancel)", 0, static_cast<int>(attack.size()), choice)) {
            finishedIssuing = true;
            return nullptr;
        }
        if (choice == 0) {
            return nullptr;
        }
        // return a new Bomb order for the chosen territory. The Bomb order will be added to the player's orders list in issueCardOrder().
        return new Bomb(player, attack[choice - 1]);
    }

    // Card BLOCKADE: ask the user which owned territory to blockade. If there are no owned territories, inform the user and return nullptr.
    if (card->getType() == CardType::BLOCKADE) {
        printTerritoryChoices(defend);
        int choice = 0;
        if (!askForNumber("Blockade which territory (0 to cancel)", 0, static_cast<int>(defend.size()), choice)) {
            finishedIssuing = true;
            return nullptr;
        }
        if (choice == 0) {
            return nullptr;
        }
        return new Blockade(player, defend[choice - 1]);
    }

    // Card AIRLIFT: ask the user for a source territory, a target territory, and how many armies to move. If there are not enough territories or armies, inform the user and return nullptr.
    if (card->getType() == CardType::AIRLIFT) {
        if (defend.size() < 2) {
            std::cout << "You need at least two territories to airlift.\n";
            return nullptr;
        }

        std::cout << "Airlift from:\n";
        printTerritoryChoices(defend);
        int sourceChoice = 0;
        if (!askForNumber("Source (0 to cancel)", 0, static_cast<int>(defend.size()), sourceChoice)) {
            finishedIssuing = true;
            return nullptr;
        }
        if (sourceChoice == 0) {
            return nullptr;
        }
        Territory* source = defend[sourceChoice - 1];

        int available = getArmiesAvailableToMove(source);
        if (available <= 0) {
            std::cout << "That territory has no armies available.\n";
            return nullptr;
        }

        std::cout << "Airlift to:\n";
        printTerritoryChoices(defend);
        int targetChoice = 0;
        if (!askForNumber("Target (0 to cancel)", 0, static_cast<int>(defend.size()), targetChoice)) {
            finishedIssuing = true;
            return nullptr;
        }
        if (targetChoice == 0) {
            return nullptr;
        }
        Territory* target = defend[targetChoice - 1];

        if (target->getId() == source->getId()) {
            std::cout << "Source and target must be different.\n";
            return nullptr;
        }

        int armyCount = 0;
        if (!askForNumber("How many armies", 1, available, armyCount)) {
            finishedIssuing = true;
            return nullptr;
        }

        movedSourceId = source->getId();
        movedArmies = armyCount;
        return new Airlift(player, source, target, armyCount);
    }

    // Card DIPLOMACY: ask the user which enemy player to negotiate with. If there are no neighbouring enemy players, inform the user and return nullptr.
    if (card->getType() == CardType::DIPLOMACY) {
        // Collect each distinct enemy player that owns a territory next to ours.
        std::vector<Player*> opponents;
        for (size_t i = 0; i < attack.size(); i++) {
            Player* owner = attack[i]->getOwner();
            if (owner == nullptr || owner == player) {
                continue;
            }
            bool alreadyListed = false;
            for (size_t j = 0; j < opponents.size(); j++) {
                if (opponents[j] == owner) {
                    alreadyListed = true;
                }
            }
            if (!alreadyListed) {
                opponents.push_back(owner);
            }
        }

        if (opponents.empty()) {
            std::cout << "There is no neighbouring player to negotiate with.\n";
            return nullptr;
        }

        for (size_t i = 0; i < opponents.size(); i++) {
            std::cout << "  " << (i + 1) << ") " << opponents[i]->getName() << "\n";
        }
        int choice = 0;
        if (!askForNumber("Negotiate with (0 to cancel)", 0, static_cast<int>(opponents.size()), choice)) {
            finishedIssuing = true;
            return nullptr;
        }
        if (choice == 0) {
            return nullptr;
        }
        return new Negotiate(player, opponents[choice - 1]);
    }

    // CardType::REINFORCEMENT (no effect defined in this project) or anything unknown.
    std::cout << "That card cannot be played in this game.\n";
    return nullptr;
}

vector<Territory*> HumanPlayerStrategy::toAttack()  {
    // Implementation for human player attack selection
    return findAdjacentEnemyTerritories();
}
vector<Territory*> HumanPlayerStrategy::toDefend()  {
    return findOwnedTerritories();
}

ostream& operator<<(ostream& out, const HumanPlayerStrategy& hps) {
    out << "Human Player Strategy";
    return out;
}




// ==================================================
// CheaterPlayerStrategy
// ==================================================
CheaterPlayerStrategy::CheaterPlayerStrategy(Player* player) : PlayerStrategy(player) {}
CheaterPlayerStrategy::CheaterPlayerStrategy(const CheaterPlayerStrategy& other) : PlayerStrategy(other) {}
PlayerStrategy* CheaterPlayerStrategy::clone() const  {
    return new CheaterPlayerStrategy(*this);
}
CheaterPlayerStrategy& CheaterPlayerStrategy::operator=(const CheaterPlayerStrategy& other) {
    if (this != &other) {
        PlayerStrategy::operator=(other);
    }
    return *this;
}
CheaterPlayerStrategy::~CheaterPlayerStrategy() {}


// Conquers every adjacent territory directly (no order, no battle), only once per turn.
// The first call of the turn does the conquests and returns true; later calls return false.
bool CheaterPlayerStrategy::issueOrder() {
    if (finishedIssuing || player == nullptr) {
        return false;
    }

    // only once per turn
    finishedIssuing = true;

    // list the targets BEFORE conquering, so the territories conquered now do not extend the reach
    std::vector<Territory*> targets = toAttack();
    bool conqueredSomething = false;

    // For each target territory, if it has an owner, transfer it to the cheater player. If it has no owner, simply add it to the cheater player's territories.
    for (size_t i = 0; i < targets.size(); i++) {
        // Get the target territory and its current owner
        Territory* target = targets[i];
        Player* previousOwner = target->getOwner();

        // If the target territory is already owned by the cheater player, skip it.
        if (previousOwner == player) {
            continue;
        }

        if (previousOwner != nullptr) {
            // take the territory away from its current owner
            if (previousOwner->transferTerritoryTo(*player, target->getId())) {
                std::cout << player->getName() << " (Cheater) conquered "
                          << target->getName() << std::endl;
                conqueredSomething = true;
            }
        } else {
            // a territory without an owner simply joins the cheater
            player->addTerritory(target);
            std::cout << player->getName() << " (Cheater) conquered "
                      << target->getName() << std::endl;
            conqueredSomething = true;
        }
    }

    if (conqueredSomething) {
        // a conquest earns the card of the turn
        player->flagConquest();
    }

    return conqueredSomething;
}
vector<Territory*> CheaterPlayerStrategy::toAttack()  {
    return findAdjacentEnemyTerritories();
}
vector<Territory*> CheaterPlayerStrategy::toDefend()  {
    return findOwnedTerritories();
}



ostream& operator<<(ostream& out, const CheaterPlayerStrategy& cps) {
    out << "Cheater Player Strategy";
    return out;
}
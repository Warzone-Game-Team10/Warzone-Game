#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <utility>
#include <algorithm>

#include "Player.h"
#include "Cards.h"
#include "Orders.h"
#include "Map.h"
#include "CardOrderIntegration.h"
#include "PlayerStrategies.h"


using namespace std;

//==========BASIC FUNCTION==================
//constructor
Player::Player()
    : name("Unnamed"), territoryList(), cardHand(new Hand(nullptr)), ordersList(new OrdersList()) {
        //default to human player strategy
        strategy = new HumanPlayerStrategy(this);
}

 Player::Player(string playerName, Hand* cardHand, OrdersList* ordersList, vector<Territory*> territoryList)
 : name(playerName), territoryList(), cardHand(nullptr), ordersList(nullptr) {
    
    this->name=playerName;
  
    //Hand
    if (cardHand != nullptr) {
        this->cardHand = new Hand(*cardHand);
    } else {
        this->cardHand = new Hand(nullptr);
    }
    
    //Order
    if (ordersList != nullptr) {
        this->ordersList = new OrdersList(*ordersList);
    } else {
        this->ordersList = new OrdersList();
    }

    //addTerritories to the List
    int count = territoryList.size();
    for (int i = 0; i < count; i++) {
        addTerritory(territoryList[i]);
    }

    //default to human player strategy
    strategy = new HumanPlayerStrategy(this);
    
} 

//copy constructor (deep copy)
Player::Player(const Player& original) : Player(original, this) {}

// Assignment can prepare the copies before giving them to the destination player.
Player::Player(const Player& original, Player* destination)
    : name(original.name), territoryList(), cardHand(nullptr), ordersList(nullptr),
      reinforcementPool(original.reinforcementPool),
      reservedReinforcementArmies(original.reservedReinforcementArmies),
      conqueredThisTurn(original.conqueredThisTurn),
      conquestCardAwardedThisTurn(original.conquestCardAwardedThisTurn) {

    
    // These temporary copies clean themselves up if any step fails.
    // Store them here only after the whole copy is ready.
    auto handCopy = std::make_unique<Hand>(*original.cardHand);
    auto ordersCopy = std::make_unique<OrdersList>(*original.ordersList);
    std::vector<std::unique_ptr<Territory>> ownedCopies;
    std::unordered_map<int, Territory*> territoryCopies;
    ownedCopies.reserve(original.territoryList.size());
    territoryList.reserve(original.territoryList.size());
    for (const Territory* territory : original.territoryList) {
        auto copy = std::make_unique<Territory>(*territory);
        copy->setOwner(destination);
        territoryCopies.emplace(territory->getId(), copy.get());
        territoryList.push_back(copy.get());
        ownedCopies.push_back(std::move(copy));
    }

    // Copy every territory first, then connect the copies to their neighbours.
    // For example, copied Ontario should point to copied Quebec.
    for (const auto& territory : ownedCopies) {
        std::vector<Territory*> neighbours = territory->getAdjacentTerritories();
        for (Territory*& neighbour : neighbours) {
            if (neighbour != nullptr) {
                const auto found = territoryCopies.find(neighbour->getId());
                if (found != territoryCopies.end()) {
                    neighbour = found->second;
                }
            }
        }
        territory->setAdjacentTerritories(std::move(neighbours));
    }
    ordersCopy->rebindContext(&original, destination, territoryCopies);

    // Deep copy of the strategy
    // clone() points the copied strategy at the player that keeps the copy
    PlayerStrategy* strategyCopy = nullptr;
    if (original.strategy != nullptr) {
        strategyCopy = original.strategy->clone();
        strategyCopy->setPlayer(destination);
    }


    cardHand = handCopy.release();
    ordersList = ordersCopy.release();
    strategy = strategyCopy;
    for (std::size_t index = 0; index < ownedCopies.size(); ++index) {
        territoryList[index] = ownedCopies[index].release();
    }
}

//assignment operator
Player& Player::operator=(const Player& other) {
    if (this != &other) {
        // Finish making the new copy before replacing this player's current data.
        Player replacement(other, this);
        name.swap(replacement.name);
        territoryList.swap(replacement.territoryList);
        std::swap(ownsTerritoryObjects, replacement.ownsTerritoryObjects);
        std::swap(strategy, replacement.strategy);
        std::swap(cardHand, replacement.cardHand);
        std::swap(ordersList, replacement.ordersList);
        std::swap(reinforcementPool, replacement.reinforcementPool);
        std::swap(reservedReinforcementArmies, replacement.reservedReinforcementArmies);
        std::swap(conqueredThisTurn, replacement.conqueredThisTurn);
        std::swap(conquestCardAwardedThisTurn, replacement.conquestCardAwardedThisTurn);
        negotiatedPlayers.clear();
        // The temporary player now holds the old data and deletes it when this function ends.
    }
    return *this;
}

//destructor
Player::~Player(){
    resetTurnState();
    delete cardHand;
    delete ordersList;
    delete strategy;
    strategy = nullptr;
    deleteAllTerritories();
}

// =========HELPER functions =======

// Deletes every Territory object in player's list, then empties the list. Used by the destructor and by operator=.
void Player::deleteAllTerritories() {
    //destroy the objects the address is poitnting to
    int count = territoryList.size();
    for (int i = 0; i < count; i++) {
        if (ownsTerritoryObjects) delete territoryList[i];
    }
    territoryList.clear(); //remove the addresse from the vector list
}

//add territories to the territory list data member
void Player::addTerritory(Territory* territory) {
    if (territory == nullptr) {
        return;   // ignore bad input instead to avoid crashing later
    }
    if (ownsTerritory(territory)) {
        return;   // if player already have this country return
    }
    
    if (!ownsTerritoryObjects) {
        territory->setOwner(this);
        territoryList.push_back(territory);
        return;
    }
    Territory* copy = new Territory(*territory);   // DEEP COPY
    copy->setOwner(this);
    territoryList.push_back(copy);//add the deep copy to the player's list
}

bool Player::transferTerritoryTo(Player& recipient, int territoryId) {
    if (&recipient == this) return false;
    for (auto it = territoryList.begin(); it != territoryList.end(); ++it) {
        Territory* territory = *it;
        if (territory && territory->getId() == territoryId) {
            if (recipient.ownsTerritory(territory)) return false;
            // Reserve before modifying either player; retain the original pointer.
            recipient.territoryList.reserve(recipient.territoryList.size() + 1);
            recipient.territoryList.push_back(territory);
            territoryList.erase(it);
            territory->setOwner(&recipient);
            return true;
        }
    }
    return false;
}

bool Player::hasConqueredThisTurn() const { return conqueredThisTurn; }
void Player::resetConqueredThisTurn() { conqueredThisTurn = false; conquestCardAwardedThisTurn = false; }
void Player::flagConquest() { conqueredThisTurn = true; }
// A conquest earns one card, regardless of how many territories were captured.
bool Player::awardConquestCard(Deck& deck) {
    if (!conqueredThisTurn || conquestCardAwardedThisTurn || !cardHand) return false;
    Card* reward = deck.draw();
    if (!reward) return false;
    cardHand->addCard(reward);
    conquestCardAwardedThisTurn = true;
    return true;
}

void Player::negotiateWith(Player& other) {
    if (&other == this) return;
    if (!isNegotiatingWith(other)) negotiatedPlayers.push_back(&other);
    if (!other.isNegotiatingWith(*this)) other.negotiatedPlayers.push_back(this);
}

bool Player::isNegotiatingWith(const Player& other) const {
    return std::find(negotiatedPlayers.begin(), negotiatedPlayers.end(), &other) != negotiatedPlayers.end();
}

void Player::resetTurnState() {
    resetConqueredThisTurn();
    for (Player* other : negotiatedPlayers) {
        if (!other) continue;
        auto& links = other->negotiatedPlayers;
        links.erase(std::remove(links.begin(), links.end(), this), links.end());
    }
    negotiatedPlayers.clear();
}


//check if player own this territory
bool Player::ownsTerritory(Territory* territory) {
    int count = territoryList.size();
    for (int i = 0; i < count; i++) {
        //if Ids match thenthe player already owns it
        if (territoryList[i]->getId() == territory->getId()) {
            return true;
        }
    }
    return false;
}


//======stragery pattern
// ====== Strategy pattern: the Player contains no decision logic ======

bool Player::issueOrder() {
    if (strategy == nullptr) {
        return false;
    }
    return strategy->issueOrder();
}

std::vector<Territory*> Player::toAttack() {
    if (strategy == nullptr) {
        return std::vector<Territory*>();
    }
    return strategy->toAttack();
}

std::vector<Territory*> Player::toDefend() {
    if (strategy == nullptr) {
        return std::vector<Territory*>();
    }
    return strategy->toDefend();
}

// Called by the GameEngine at the start of each issuing phase.
void Player::beginIssuing() {
    reservedReinforcementArmies = 0;
    if (strategy != nullptr) {
        strategy->beginIssuing();
    }
}

PlayerStrategy* Player::getStrategy() const {
    return strategy;
}

void Player::notifyAttacked() {
    if (strategy != nullptr && strategy->shouldBecomeAggressiveWhenAttacked()) {
        AggressivePlayerStrategy aggressive(this);
        setStrategy(&aggressive);
    }
}



//creates an Order object and puts it in the player's list of orders
// Issue a normal order.
// Card-only orders must be issued through their corresponding cards.
// Compatibility entry point: all decisions still go through the player's policy.
void Player::issueOrder(string orderType) {
    if (orderType == "Deploy" || orderType == "Advance") issueOrder();
    else cout << "Card orders must be issued by playing a card.\n";
}

void Player::bindMapTerritories(const vector<Territory*>& territories) {
    deleteAllTerritories();
    ownsTerritoryObjects = false;
    territoryList = territories;
    for (Territory* territory : territoryList) territory->setOwner(this);
}

bool Player::borrowsMapTerritories() const { return !ownsTerritoryObjects; }

void Player::releaseNeutralTerritory(Territory* territory) {
    territoryList.erase(std::remove(territoryList.begin(), territoryList.end(), territory), territoryList.end());
    territory->setOwner(nullptr);
}



// Adding armies ( during the reinforcement phase )
void Player::addReinforcementArmies(int armies){

    // make sure the user inputs a positive number... 
    if(armies > 0 ){
        reinforcementPool += armies;
    }
}


// Removing armies ( used later on when we issue deploy orders )
void Player:: removeReinforcementArmies(int armies){

    // if user inputs a negative number ==> output nothing..
    if(armies <= 0){
        return; 
    }

    // if user inputs a number thats greater than the number of armies they own 
    if(armies > reinforcementPool){
        std::cout << " Error: not enough armies in the reinforcement pool! \n ";
        return; 
    }

    // otherwise, if they input a valid number : 
    reinforcementPool -= armies; 
}


//=======GETTER
//most of these getter returns a point, so make sure to dereferencing it with * to gives the object at that address its pointing at
string Player::getName(){
    return name;
};

vector<Territory*> Player::getTerritoryList(){
    return territoryList;
};

Hand* Player::getCardHand(){
    return cardHand;
};

OrdersList* Player::getOrdersList(){
    return ordersList;
};

//========SETTER
void Player::setName(string name){
    this->name = name;
};

void Player::setTerritoryList(vector<Territory*> newTerritories) {
    
    //Remember the old territories, then empty our list.
    vector<Territory*> oldTerritories = territoryList;
    territoryList.clear(); //remove all elements in the vector

    //Deep-copy the new ones in addTerritory() 
    int count = newTerritories.size();
    for (int i = 0; i < count; i++) {
        addTerritory(newTerritories[i]);
    }

    //delete the old territories the player owned.
    int oldCount = oldTerritories.size();
    for (int i = 0; i < oldCount; i++) {
        if (ownsTerritoryObjects) delete oldTerritories[i];
    }
}

void Player::setCardHand(Hand* newCardHand){
    if (newCardHand == nullptr || newCardHand == cardHand) {
        return;                                  // ignore bad input or "set to itself"
    }
    Hand* oldCardHand = cardHand;          // remember the old list
    cardHand = new Hand(*newCardHand);     // deep copy FIRST (a separate list)
    delete oldCardHand;                            // then free the old one
};

void Player::setOrdersList(OrdersList* newOrders){
    if (newOrders == nullptr || newOrders == ordersList) {
        return;                                  // ignore bad input or "set to itself"
    }
    OrdersList* oldOrders = ordersList;          // remember the old list
    ordersList = new OrdersList(*newOrders);     // deep copy FIRST (a separate list)
    delete oldOrders;                            // then free the old one
};                



// Gives the player a deep copy of newStrategy, bound to this player
void Player::setStrategy(PlayerStrategy* newStrategy) {
    // if the new strategy is the same as the current one, do nothing.
    if (newStrategy == strategy) {
        return;
    }

    PlayerStrategy* replacement = nullptr;

    // If the new strategy is not null, clone it and point the copy at this player.
    if (newStrategy != nullptr) {
        replacement = newStrategy->clone();   // copy first, so a failure changes nothing
        replacement->setPlayer(this);
    }

    // Swap the new strategy in and delete the old one.
    PlayerStrategy* oldStrategy = strategy;
    strategy = replacement;
    delete oldStrategy;
}


//======Like the toString
ostream& operator<<(ostream& out, const Player& p) {
    //print out name
    out << "===== Player " << p.name << " =====" << endl

        << "Territories (" << p.territoryList.size() << "):" <<endl;
    
    //print out territories
    int count = p.territoryList.size();
    for (int i = 0; i < count; i++) {
        out <<i<<") "<<p.territoryList[i]->getName()<<endl;
    }

    //print out Cards
    //p.cardHand returns a point and dereferencing it with * to gives the object at that address its pointing at
    out << "Cards: " << *(p.cardHand) << endl;

    //print out orders
    out << "Orders:" << endl << *(p.ordersList) << endl;
    return out;
}

// Reinforcement pool management
int Player::getReinforcementPool() const {
    return reinforcementPool;
}

// retursn reinforcement pool - armies already reserved for deploy orders
int Player::getAvailableReinforcementArmies() const {
    return std::max(0, reinforcementPool - reservedReinforcementArmies);
}
// return nb of armies if still available + increase reserved count on success
bool Player::reserveReinforcementArmies(int armies) {
    if (armies <= 0 || armies > getAvailableReinforcementArmies()) {
        return false;
    }
    reservedReinforcementArmies += armies;
    return true;
}

void Player::setReinforcementPool(int armies) {
    if (armies >= 0) {
        reinforcementPool = armies;
    }
}

bool Player::useReinforcements(int armies) {
    if (armies <= 0 || armies > reinforcementPool) {
        return false;
    }
    reinforcementPool -= armies;
    return true;
}

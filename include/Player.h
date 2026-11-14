
#pragma once

#include <stdio.h>
#include <vector>
#include <string>

#include "Cards.h"
#include "Orders.h"
#include "Map.h"

using namespace std;

class Player {
private:
    // Standalone players own copies; engine players borrow the map's territories.
    bool ownsTerritoryObjects = true;
    int committedDeployArmies = 0;
    int issuingStep = 0;
    bool finishedIssuing = false;
    string name;
    vector<Territory*> territoryList;
    Hand* cardHand;
    OrdersList* ordersList;

    // NEW: Available armies for deployment
    int reinforcementPool = 0;
    bool conqueredThisTurn = false;
    bool conquestCardAwardedThisTurn = false;
    std::vector<Player*> negotiatedPlayers;

    // Build the copies for the player that will keep them.
    Player(const Player& original, Player* destination);

public:
    //==========BASIC FUNCTION==================

    // Constructors
    Player();
    Player(string name, Hand* cardHand,
           OrdersList* ordersList,
           vector<Territory*> territoryList);

    // Copy constructor
    Player(const Player& p);

    // Assignment operator
    Player& operator=(const Player& other);

    // Destructor
    ~Player();

    //=======HELPERS=======

    // Deletes every Territory object in player's list,
    // then empties the list.
    void deleteAllTerritories();

    // Add territories to the player's list
    void addTerritory(Territory* territory);

    // Check if player owns this territory
    bool ownsTerritory(Territory* territory);
    // Move ownership of the existing territory object (without invalidating pointers).
    bool transferTerritoryTo(Player& recipient, int territoryId);
    bool hasConqueredThisTurn() const;
    void resetConqueredThisTurn();
    void flagConquest();
    bool awardConquestCard(Deck& deck);
    void negotiateWith(Player& other);
    bool isNegotiatingWith(const Player& other) const;
    void resetTurnState();

    //------------ ASSIGNMENT 2 : PART 3 ------------------------------

    void addReinforcementArmies(int armies);

    void removeReinforcementArmies(int armies);

    //==========ASSIGNMENT FUNCTION==================

    // Return territories that are to be defended
    vector<Territory*> toDefend();

    // Return territories that are to be attacked
    vector<Territory*> toAttack();

    // Create an Order object and add it to the player's orders list
    void issueOrder(string orderType);
    // Issue one decision per call; false means this player has passed for the turn.
    bool issueOrder();
    void beginIssuing();
    // Replace owned copies with borrowed map territories (engine integration only).
    void bindMapTerritories(const vector<Territory*>& territories);
    bool borrowsMapTerritories() const;
    void releaseNeutralTerritory(Territory* territory);

    //==========GETTER FUNCTION==================

    string getName();
    vector<Territory*> getTerritoryList();
    Hand* getCardHand();
    OrdersList* getOrdersList();

    // NEW: Get available reinforcement armies
    int getReinforcementPool() const;

    //==========SETTER FUNCTION==================

    void setName(string name);
    void setTerritoryList(vector<Territory*> territoryList);
    void setCardHand(Hand* cardHand);
    void setOrdersList(OrdersList* ordersList);

    // NEW: Reinforcement pool management
    void setReinforcementPool(int armies);
    bool useReinforcements(int armies);

    //=====To String========

    friend ostream& operator<<(ostream& out, const Player& player);
};





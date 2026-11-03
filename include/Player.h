
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
    string name;
    vector<Territory*> territoryList;
    Hand* cardHand;
    OrdersList* ordersList;

    // NEW: Available armies for deployment
    int reinforcementPool = 0;
    bool conqueredThisTurn = false;

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

    //==========ASSIGNMENT FUNCTION==================

    // Return territories that are to be defended
    vector<Territory*> toDefend();

    // Return territories that are to be attacked
    vector<Territory*> toAttack();

    // Create an Order object and add it to the player's orders list
    void issueOrder(string orderType);

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





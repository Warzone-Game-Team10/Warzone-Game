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

    public:
    //==========BASIC FUNCTION==================
        //constructors
        Player();
        Player(string name, Hand* cardHand, OrdersList* ordersList, vector<Territory*> territoryList);
         // copy constructor
        Player(const Player& p);     
        // assignment operator             
        Player& operator=(const Player& other);   
        // destructor     
        ~Player();  

    //=======HELPERS=======
        // Deletes every Territory object in player's list, then empties the list. 
        void deleteAllTerritories();
        //add territories to the players list
        void addTerritory(Territory* territory);
        //check if player own this territory
        bool ownsTerritory(Territory* territory);

    //==========ASSIGNMENT FUNCTION==================
        //return a list of territories that are to be defended
        vector<Territory*> toDefend();
        //returns a list of territories that are to be attacked.
        vector<Territory*> toAttack();
        //creates an Order object and puts it in the player's list of orders
        void issueOrder(string orderType);

    //==========GETTER FUNCTION==================
        string getName();
        vector<Territory*> getTerritoryList();
        Hand* getCardHand();
        OrdersList* getOrdersList();

    //==========SETTER FUNCTION==================
        void setName(string name);
        void setTerritoryList(vector<Territory*> territoryList);
        void setCardHand(Hand* cardHand);
        void setOrdersList(OrdersList* ordersList);
     
    //=====To String======== 
    // Printing:   std::cout << somePlayer;   (like overriding toString()).
    friend ostream& operator<<(ostream& out, const Player& player);
};





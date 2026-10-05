#pragma once
#include <stdio.h>
#include <vector>
#include <string>

#include "Cards.h"
#include "Order.h"
#include "Map.h"

using namespace std;


class Player {
    private:
        string name;
        vector<Territory*> territoryList;
        vector<Territory*> attackList;
        Cards* cardHand;
        OrdersList* ordersList; 

    public:
        //constructors
        Player();
        Player(string name, Cards* cardHand, OrdersList* ordersList, vector<Territory*> territoryList, vector<Territory*> attackList);
        

        //return a list of territories that are to be defended
        vector<Territory*> toDefend();

        //returns a list of territories that are to be attacked.
        vector<Territory*> toAttack();

        //creates an Order object and puts it in the player's list of orders
        void issueOrder();


        //getters
        string getName();
        vector<Territory*> getTerritoryList();
        vector<Territory*> getAttackList();
        Cards* getCardHand();
        OrdersList* getOrdersList();

        //setters
        void setName(string name);
        void setTerritoryList(vector<Territory*> territoryList);
        void setAttackList(vector<Territory*> attackList);
        void setCardHand(Cards* cardHand);
        void setOrdersList(OrdersList* ordersList);

};





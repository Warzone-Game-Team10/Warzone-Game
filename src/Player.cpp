#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>

#include "Player.h"
#include "Cards.h"
#include "Orders.h"
#include "Map.h"


using namespace std;

//==========BASIC FUNCTION==================
//constructor
Player::Player()
    : name("Unnamed"), territoryList(), cardHand(new Hand(nullptr)), ordersList(new OrdersList()) {}

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
    
} 

//copy constructor (deep copy)
Player::Player(const Player& p){
    this->name= p.name;
    this->cardHand = new Hand(*(p.cardHand));
    this->ordersList = new OrdersList(*(p.ordersList));

     // deep copy of each territory
    int count = p.territoryList.size();
    for (int i = 0; i < count; i++) {
        addTerritory(p.territoryList[i]);        
    }
}

//assignment operator
Player& Player::operator=(const Player& other) {
   
    if (this != &other) {
        // Free what we currently own.
        delete cardHand;
        delete ordersList;
        deleteAllTerritories();
 
        // Step 2: deep copy from the other player.
        name = other.name;
        cardHand = new Hand(*(other.cardHand));
        ordersList = new OrdersList(*(other.ordersList));
        
        //deep copy territories
        int count = other.territoryList.size();
        for (int i = 0; i < count; i++) {
            addTerritory(other.territoryList[i]);
        }
    }

    //return the current ("this") 
    return *this; 
}

//destructor
Player::~Player(){
    delete cardHand;
    delete ordersList;
    deleteAllTerritories();
}

// =========HELPER functions =======

// Deletes every Territory object in player's list, then empties the list. Used by the destructor and by operator=.
void Player::deleteAllTerritories() {
    //destroy the objects the address is poitnting to
    int count = territoryList.size();
    for (int i = 0; i < count; i++) {
        delete territoryList[i];
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
    
    Territory* copy = new Territory(*territory);   // DEEP COPY
    copy->setOwner(this);
    territoryList.push_back(copy);//add the deep copy to the player's list
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

//return a list of territories that player OWNs that are to be defended
vector<Territory*> Player::toDefend(){
    return territoryList;
}

//returns a list of territories that are to be attacked.
vector<Territory*> Player::toAttack(){

    //store list of territories to attack
    std::vector<Territory*> result;
 
    int myCount = territoryList.size();
    for (int i = 0; i < myCount; i++) {
        //Look at each territory I own and see its neighbours with getAdjacentTerritories() from map file.
        std::vector<Territory*> neighbours = territoryList[i]->getAdjacentTerritories();
 
        //check if the neighbours are not repeasting or part of the territory list already
        int neighbourCount = neighbours.size();
        for (int j = 0; j < neighbourCount; j++) {
            Territory* candidate = neighbours[j];
 
            // ignore a null neighbour.
            if (candidate == nullptr) {
                continue;
            }
 
            // Check if player already own the territory is yes then skip
            if (ownsTerritory(candidate)) {
                continue;   
            }
 
            // check if territory is already in the list of results. if yes then don't add else add to the list result
            bool alreadyListed = false;
            int resultCount = result.size();
            for (int k = 0; k < resultCount; k++) {
                if (result[k]->getId() == candidate->getId()) {
                    alreadyListed = true;
                }
            }
 
            if (alreadyListed == false) {
                result.push_back(candidate);
            }
        }
    }
    return result;
    
}

//creates an Order object and puts it in the player's list of orders
void Player::issueOrder(string orderType){
    Order* newOrder = nullptr;      // will point at whichever order we build

    if (orderType == "Deploy") {
        newOrder = new Deploy();
    } else if (orderType == "Advance") {
        newOrder = new Advance();
    } else if (orderType == "Bomb") {
        newOrder = new Bomb();
    } else if (orderType == "Blockade") {
        newOrder = new Blockade();
    } else if (orderType == "Airlift") {
        newOrder = new Airlift();
    } else if (orderType == "Negotiate") {
        newOrder = new Negotiate();
    } else {
        cout << "Unknown order type: " << orderType << endl;
        return;  // build nothing, add nothing
    }

    ordersList->addOrder(newOrder); // the list now OWNS this order
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
        delete oldTerritories[i];
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

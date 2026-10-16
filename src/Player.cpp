#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>

#include "Player.h"
#include "Cards.h"
#include "Order.h"
#include "Map.h"


using namespace std;

//==========BASIC FUNCTION==================
//constructor
Player::Player()
    : name("Unnamed"), territoryList(), cardHand(new Cards()), ordersList(new OrdersList()) {}

 Player::Player(string name, Cards* cardHand, OrdersList* ordersList, vector<Territory*> territoryList){
    
    this->name=name;
    this->cardHand=new Cards();
    this->ordersList = new OrdersList();

    //addTerritories to the List
    int count = territoryList.size();
    for (int i = 0; i < count; i++) {
        addTerritory(territoryList[i]);
    }
    
}

//copy constructor (deep copy)
Player::Player(const Player& p){
    this->name= p.name;
    this->cardHand = new Cards(*(p.cardHand));
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
        cardHand = new Cards(*(other.cardHand));
        ordersList = new OrdersList(*(other.ordersList));
        
        //deep copy territories
        int count = other.territoryList.size();
        for (int i = 0; i < count; i++) {
            addTerritory(other.territoryList[i]);
        }
    }

    //return the current ("this") object as a pointer ?
    return *this; 
}

//destructor
Player::~Player(){
    delete cardHand;
    delete ordersList;
    deleteAllTerritories();
}

// =========HELPER

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
        return;   // if playe already have this country return
    }
    
    Territory* copy = new Territory(*territory);   // DEEP COPY
    copy->setOwner(this);
    territoryList.push_back(copy);//add the deep copy to the player's list
}

//check if player own this territory
bool Player::ownsTerritory(Territory* territory) {
    int count = territoryList.size();
    for (int i = 0; i < count; i++) {
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
 
        //check if the neighbours are not repeasting or part of the territory list
        int neighbourCount = neighbours.size();
        for (int j = 0; j < neighbourCount; j++) {
            Territory* candidate = neighbours[j];
 
            // ignore a null neighbour.
            if (candidate == nullptr) {
                continue;
            }
 
            // Check if player already own the territory is yes the skip
            if (ownsTerritory(candidate)) {
                continue;   
            }
 
            // check if territory is already in the list of results. if yes then don't read else add to the list result
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
void Player::issueOrder(){
    Order* order =new Order();
    ordersList->add(order);
    cout<< "===a new order was added: ===" << endl;
}


//=======GETTER
string Player::getName(){
    return name;
};
vector<Territory*> Player::getTerritoryList(){
    return territoryList;
};
Cards* Player::getCardHand(){
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
void Player::setCardHand(Cards* cardHand){
    this->cardHand = cardHand;
};
void Player::setOrdersList(OrdersList* ordersList){
    this->ordersList = ordersList;
};                

//======String
std::ostream& operator<<(std::ostream& out, const Player& p) {
    out << "Player " << p.name
        << " [territories size: " << p.territoryList.size()
         << "]";
    return out;
}
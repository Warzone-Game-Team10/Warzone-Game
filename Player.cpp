#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>

#include "Player.h"
#include "Cards.h"
#include "Order.h"
#include "Map.h"


using namespace std;

//constructor
Player::Player(){

    this->name="Bob Jonh";
    this->cardHand=new Cards();
    this->ordersList = new OrdersList();

    // arbitrary territories to defend
    this->territoryList.push_back(new Territory("Alaska"));
    this->territoryList.push_back(new Territory("Canada")); 
    
    // arbitrary territories to attack
    this->attackList.push_back(new Territory("USA"));
    this->attackList.push_back(new Territory("China")); 

}

 Player::Player(string name, Cards* cardHand, OrdersList* ordersList, vector<Territory*> territoryList, vector<Territory*> attackList){
    
    this->name=name;
    this->cardHand=cardHand;
    this->ordersList=ordersList;

    this->territoryList=territoryList;
    this->attackList=attackList;
    
}

//return a list of territories that are to be defended
vector<Territory*> Player::toDefend(){
    cout<< "===Territories to defend: ===" << endl;
    for (int i = 0; i < territoryList.size(); i++) {
        cout << territoryList[i]->getName() << endl;
    }
    return territoryList;
}

//returns a list of territories that are to be attacked.
vector<Territory*> Player::toAttack(){
    cout<< "===Territories to attack: ===" << endl;
    for (int i = 0; i < attackList.size(); i++) {
        cout << attackList[i]->getName() << endl;
    }
    return attackList;
}

//creates an Order object and puts it in the player's list of orders
void Player::issueOrder(){
    Order* order =new Order();
    ordersList->add(order);

}


//getters
string Player::getName(){
    return name;
};
vector<Territory*> Player::getTerritoryList(){
    return territoryList;
};
vector<Territory*> Player::getAttackList(){
    return attackList;
};
Cards* Player::getCardHand(){
    return cardHand;
};
OrdersList* Player::getOrdersList(){
    return ordersList;
};

//setters
void Player::setName(string name){
    this->name = name;
};
void Player::setTerritoryList(vector<Territory*> territoryList){
    this->territoryList = territoryList;
};
void Player::setAttackList(vector<Territory*> attackList){
    this->attackList = attackList;
};
void Player::setCardHand(Cards* cardHand){
    this->cardHand = cardHand;
};
void Player::setOrdersList(OrdersList* ordersList){
    this->ordersList = ordersList;
};                
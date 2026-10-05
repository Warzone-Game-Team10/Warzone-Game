#include <stdio.h>
#include <iostream>

#include "Player.h"
#include "Cards.h"
#include "Order.h"
#include "Map.h"

using namespace std;

void testPlayer(){

    //create a player object
    Player* player1 = new Player();

    // show player name
    cout << "Player name: " << player1->getName() << endl;

    // Change player names
    player1->setName("Alice");
    cout << "Player name after setName(): " << player1->getName() << endl;

 
    // show toDefend()
    player1->toDefend();
 
    // show toAttack()
    player1->toAttack();

    //add orders to the player's order list
    player1->issueOrder();


    // free memory
    delete player1;

}

int main(){

    testPlayer();

    return 0;
};
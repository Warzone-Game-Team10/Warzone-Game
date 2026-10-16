#include <stdio.h>
#include <iostream>

#include "Player.h"
#include "Cards.h"
#include "Order.h"
#include "Map.h"

using namespace std;

void testToDefend(Player* player, int playerNumber){
    cout<<"===================PLAYER "<< playerNumber << ": list of teritories to defend========================"<<endl;
    vector<Territory*> defendList1 =player->toDefend() ;
    for (int i = 0; i < defendList1.size(); i++) {
        cout << defendList1[i]->getName() << endl;
    }
}

void testToAttack(Player* player, int playerNumber){
    cout<<"===================PLAYER "<< playerNumber << ": list of teritories to attack========================"<<endl;
    vector<Territory*> defendList1 =player->toAttack() ;
    for (int i = 0; i < defendList1.size(); i++) {
        cout << defendList1[i]->getName() << endl;
    }
}

void testCopyConstructor( Player* player1,  vector<Territory*> defendList, Map* gameMap){
     cout<<"====================TESTING COPY CONSTRUCTOR========================"<<endl;
    //display list of player 1 territory
    cout<<"Current PLAYER 1"<<endl;
    cout<<"Territories to defend: "<<endl;
    //create, store and display the list of territroies to defend
    // vector<Territory*> defendList =player1->toDefend() ;
    for (int i = 0; i < defendList.size(); i++) {
        cout << defendList[i]->getName() << endl;
    }
    //set players name
    player1->setName("Bob");
    //display name
    cout<<"Player 1 Name: "<< player1->getName()<<endl;

    cout<<"------Now we copy player 1 into new player 3------"<<endl;

    //create player 3 using copy constructor
    Player* player3 = new Player(*player1);

    //display player 3 information
    cout<<"Current PLAYER 3"<<endl;
    cout<<"Territories to defend: "<<endl;
    //create, store and display the list of territroies to defend
    vector<Territory*> defendList3 =player3->toDefend() ;
    for (int i = 0; i < defendList3.size(); i++) {
        cout << defendList3[i]->getName() << endl;
    }
    //display name
    cout<<"Player 3 Name: "<< player3->getName()<<endl;

    //===change player 1 names
    cout<<"------Now changing Player 1 name from Bob to Steve------"<<endl;
    player1->setName("Steve");
    cout<<"Player 1 name is " << player1->getName()<<endl;
    cout<<"Player 3 name is " << player3->getName()<<endl;


    //===change player 3 territory list
    cout<<endl<<"------Now changing Player 1 territory list adding new territories------"<<endl;
     //put the territories 12-15 in player 1
    for (int i = 12; i < 15; i++)
    {
        Territory* t= gameMap->getTerritories()[i].get();
        player1->addTerritory(t);
    }

    cout<<"Player 1 New Territories to defend: "<<endl;
    //create, store and display the list of territroies to defend
    vector<Territory*> newDefendList =player1->toDefend() ;
    for (int i = 0; i < newDefendList.size(); i++) {
        cout << newDefendList[i]->getName() << endl;
    }
    
    cout<<"Player 3 Territories to defend: "<<endl;
    //create, store and display the list of territroies to defend
    for (int i = 0; i < defendList3.size(); i++) {
        cout << defendList3[i]->getName() << endl;
    }
    

    //delete
     delete player3;
}


int main(){

    //=========Read the file=======
    MapLoader* loader = new MapLoader();
    Map* gameMap = nullptr;
    try {
        //use the nyc.map for testing
        gameMap = loader->loadMap("../maps/nyc/NYC.map").release();
    } catch (std::exception& error) {
        std::cout << "Could not load map: " << error.what() << std::endl;
        delete loader;
        return 1;
    }
    delete loader;
    
    //=====create 2 players=====
    Player* player1= new Player();
    Player* player2= new Player();

    //====Distribute Territories to two players====
    int size= gameMap->getTerritories().size();

    //put the territories 1-5 in player 1
    for (int i = 0; i < 5; i++)
    {
        Territory* t= gameMap->getTerritories()[i].get();
        player1->addTerritory(t);
    }

    //put the territories 6-10 in player 2
    for (int i = 5; i < 10; i++)
    {
        Territory* t= gameMap->getTerritories()[i].get();
        player2->addTerritory(t);
    }

    

    //======TESTING====
    testToDefend(player1,1);
    testToAttack(player1,1);
    
    testToDefend(player2,2);
    testToAttack(player2,2);

    vector<Territory*> defendList1 =player1->toDefend() ;
    for (int i = 0; i < defendList1.size(); i++) {
        cout << defendList1[i]->getName() << endl;
    }
   
    testCopyConstructor(player1, defendList1, gameMap);    

   



    //delete to prevent memory leaks
    delete player1;
    delete player2;
    delete gameMap;//because loadMap returns a smart pointer we need to delete even if the "new" key word is not used

    return 1;
}
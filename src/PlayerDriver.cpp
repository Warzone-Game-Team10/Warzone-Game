#include <stdio.h>
#include <iostream>
#include <cmath> // Required for std::floor

#include "Player.h"
#include "Cards.h"
#include "Orders.h"
#include "Map.h"

using namespace std;

void testSetTerritory(Map* gameMap, Player* player1){
        //display before
        cout<<"BEFORE-SetTerriotriesList"<<endl;
        //Use GET FUNCTION to print out
        vector<Territory*> oldTerritories = player1->getTerritoryList();
        int count = oldTerritories.size();
        cout << "Territories (" << count << "):" << endl;
        for (int i = 0; i < count; i++) {
            cout << "   " << oldTerritories[i]->getName() << endl;
        }       

        //put the first 3% of the territories into player 1
        int size= floor((gameMap->getTerritories().size())*0.3);
        cout<<endl<<"Now putting the first " << size << " territories into player 1"<<endl<<endl;

        //use setter for the territory list 
        vector<Territory*> territoryList;
        for (int i = 0; i < size; i++) //get some territories form the game map
        {
            Territory* t= gameMap->getTerritories()[i].get();
            territoryList.push_back(t); //add the new territories to the list
        }
        //set the new territory list for player 3 and get the toDefend list
        player1->setTerritoryList(territoryList);

        //display after
        cout<<"AFTER-SetTerriotriesList"<<endl;
        //Use GET FUNCTION to print out
        vector<Territory*> newTerritories = player1->getTerritoryList();
        count = newTerritories.size();
        cout << "Territories (" << count << "):" << endl;
        for (int i = 0; i < count; i++) {
            cout << "   " << newTerritories[i]->getName() << endl;
        }         
    }

void testSetCardHand(Player* player1, Deck* deck){
    //display before
    cout<<"BEFORE-SetCardHand"<<endl; 
    //Use GET FUNCTION and dereferencing to print out
    cout<< *(player1->getCardHand()) <<endl;
    
    //create hand using the deck made in main()
    Hand* hand = new Hand(deck);

    cout<<endl<<"Now adding 2 cards..." <<endl <<endl;
    //add cards to the hand
    hand->addCard(deck->draw());  // Hand takes ownership of the drawn card
    hand->addCard(deck->draw());

    //set the cards to player1
    player1->setCardHand(hand);

    //display after
    cout<<"AFTER-SetCardHand"<<endl;  
    //Use GET FUNCTION and dereferencing to print out
    cout<< *(player1->getCardHand()) <<endl;

    delete hand;
}    

void testingSetOrder(Player* player1){
    //display before
    cout<<"BEFORE-SetOrder"<<endl;
    //Use GET FUNCTION and dereferencing to print out
    cout<< *(player1->getOrdersList()) <<endl;
    
    //create a order list to set the player to
    cout<<endl<<"Now adding Deploy and Advance as a order"<<endl<<endl;
    OrdersList* orderList = new OrdersList();

    //create the orders
    Order* order1 = new Deploy();
    Order* order2 = new Advance();
    //add order to the list
    orderList->addOrder(order1);
    orderList->addOrder(order2);

    //set the player to orderlist
    player1->setOrdersList(orderList);

    //display after
    cout<<"AFTER-SetOrder"<<endl;
    //Use GET FUNCTION and dereferencing to print out
    cout<< *(player1->getOrdersList()) <<endl;   

    delete orderList; //class orderList delete the order1 and order2 already
}

void testingIssueOrder(Player* player1){
    //display before and dereferencing
    cout<<"BEFORE- issueOrder"<<endl << *player1<<endl;  

    //issue one correct order and one bad order for testing
    cout<<"Now adding Bomb order and a Foo order" <<endl<<endl;
    player1->issueOrder("Deploy");
    player1->issueOrder("Foo");

    //display after and dereferencing
    cout<<endl<<"AFTER- issueOrder"<<endl << *player1<<endl;  
}

void testToDefend(Player* player){
    //get list using toDefend() and display the list of territories
    vector<Territory*> defendList1 =player->toDefend() ;
    for (int i = 0; i < defendList1.size(); i++) {
        cout << defendList1[i]->getName() << endl;
    }
}

void testToAttack(Player* player){
    //get list using toAttack() and display the list of territories
    vector<Territory*> defendList1 =player->toAttack() ;
    for (int i = 0; i < defendList1.size(); i++) {
        cout << defendList1[i]->getName() << endl;
    }
}

void testCopyConstructor( Player* player1, Map* gameMap, Deck* deck){

    //display and dereferencing player 1 
    cout<<*player1<<endl;

    //===COPY PLAYER 1===
    cout<<endl<<"------Now we copy player 1 into new player 3------"<<endl<<endl;
    //create player 3 using copy constructor
    Player* player3 = new Player(*player1);

    //display and dereferencing player 3
    cout<<*player3<<endl;

    //===CHANGE PLAYER 1 NAME (using setName())
    cout<<endl<<"------Now changing Player 1 name from to Steve------"<<endl;
    player1->setName("Steve");

    //===CHANGE PLAYER 1's territory list  
    cout<<"------Now changing Player 1 territory list------"<<endl;

    //put the first 3% of the territories into player 3 starting form the middle of the list
    int size= floor((gameMap->getTerritories().size())*0.3);
    int middle_point= floor(size/2);
    cout<<"------Now putting the first " << size << " territories into player 1, starting form index "<<middle_point<<"------"<<endl;

    //create territorylist to set the player1 to
    vector<Territory*> territoryList;
    for (int i = middle_point; i < (size + middle_point); i++) //get some territories form the game map
    {
        Territory* t= gameMap->getTerritories()[i].get();
        territoryList.push_back(t); //add the new territories to the list
    }
    //set the new territory list for player 3 and get the toDefend list
    player1->setTerritoryList(territoryList);

    //===CHANGING PLAYER 1 adding 1 order
    cout<<"------Now adding Player 1 a new order (BOMB)------"<<endl;
    player1->issueOrder("Bomb");

    //===CHANGING PLAYER 1 adding 2 cards
    cout<<"------Now giving 2 cards to Player 1------"<<endl<<endl;
    testSetCardHand(player1,deck);

    //display and dereferencing results
    cout<<endl<<"------Transformation is done. Here are the new results------"<<endl<<endl;
    cout<<*player1<<endl;
    cout<<*player3<<endl;
    

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
    
    
    //=====create players=====
    Player* player1= new Player();
    //give player a name
    player1->setName("Jane Doe");

    cout<<endl<<"==============================[TESTING: SET TERRITORY]================================================="<<endl;
    testSetTerritory(gameMap, player1);

    cout<<endl<<"==============================[TESTING: SET CARD]================================================="<<endl;
    //create desk so we can use it for hands
    Deck* deck = new Deck();
    testSetCardHand(player1, deck);
    //make player 1 play all their cards.
    cout<<endl<<endl<<"----Make player 1 play all their cards---"<<endl;
    player1->getCardHand()->playAll(player1);

    cout<<endl<<"==============================[TESTING: SET ORDER]================================================="<<endl;
    testingSetOrder(player1);
    
    cout<<endl<<"==============================[TESTING: ISSUE ORDER]================================================="<<endl;
    testingIssueOrder(player1);

    cout<<endl<<"==============================[TESTING: TO DEFEND]================================================="<<endl;
    testToDefend(player1);

    cout<<endl<<"==============================[TESTING: TO ATTACK]================================================="<<endl;
    testToAttack(player1);
    
    cout<<endl<<"==============================[TESTING: COPY CONTRACTOR]================================================="<<endl;
    testCopyConstructor(player1, gameMap, deck);    

    cout<<endl<<"==============================[TESTING IS OVER. THANK YOU!]================================================="<<endl<<endl;

    //delete to prevent memory leaks
    delete loader;
    delete player1;
    delete deck;
    delete gameMap;//because loadMap returns a smart pointer we need to delete even if the "new" key word is not used
    return 1;
}
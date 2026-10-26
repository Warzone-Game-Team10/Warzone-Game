#include <stdio.h>
#include <iostream>
#include <cmath> // Required for std::floor
#include <filesystem>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

#include "PlayerDriver.h"
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
    cout<<"BEFORE- issueOrder"<<endl << *(player1->getOrdersList())<<endl;  

    //issue one correct order and one bad order for testing
    cout<<"Now adding Bomb order and a Foo order" <<endl<<endl;
    player1->issueOrder("Bomb");
    player1->issueOrder("Foo");

    //display after and dereferencing
    cout<<endl<<"AFTER- issueOrder"<<endl << *(player1->getOrdersList())<<endl;  
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




namespace {

void checkPlayerCopy(bool condition, const string& message) {
    if (!condition) {
        throw runtime_error("Player copy check failed: " + message);
    }
}

string orderText(Order* order) {
    ostringstream text;
    text << *order;
    return text.str();
}

// Hide execution messages during these tests. A failed check still explains what went wrong.
void executeForPlayerCopy(Order* order) {
    ostringstream output;
    auto previous = cout.rdbuf(output.rdbuf());
    try {
        order->execute();
    } catch (...) {
        cout.rdbuf(previous);
        throw;
    }
    cout.rdbuf(previous);
}

// Check that construction and assignment leave Bob usable after Alice is deleted.
// Bob's orders and connections between his territories must still work.
void testPlayerCopiesSurviveSource(bool assignment, bool ownedReferences) {
    const string scenario = string(assignment ? "assignment" : "copy construction") +
                            (ownedReferences ? " with owned links" : " with map aliases");
    Deck deck;
    Continent continent("Canada", 5);
    Player carol;
    carol.setName("Carol");
    Territory manitoba(3, "Manitoba", &continent);
    manitoba.setOwner(&carol);
    unique_ptr<Player> bob;
    vector<string> storedText;
    {
        Player alice;
        alice.setName("Alice");
        Territory ontario(1, "Ontario", &continent);
        Territory quebec(2, "Quebec", &continent);
        ontario.setArmyCount(10);
        ontario.addAdjacentTerritory(&quebec);
        ontario.addAdjacentTerritory(&manitoba);
        quebec.addAdjacentTerritory(&ontario);
        alice.addTerritory(&ontario);
        alice.addTerritory(&quebec);
        // Bob's orders must use the four armies copied from Alice's Ontario,
        // even if the original order points to the map's Ontario with ten armies.
        alice.getTerritoryList()[0]->setArmyCount(4);
        Territory* orderOntario = ownedReferences ? alice.getTerritoryList()[0] : &ontario;
        Territory* orderQuebec = ownedReferences ? alice.getTerritoryList()[1] : &quebec;
        if (ownedReferences) {
            orderOntario->setAdjacentTerritories({orderQuebec, &manitoba});
            orderQuebec->setAdjacentTerritories({orderOntario});
        }
        Hand hand(&deck);
        hand.addCard(new Card(CardType::BOMB));
        alice.setCardHand(&hand);
        OrdersList* orders = alice.getOrdersList();
        orders->addOrder(new Deploy(&alice, orderOntario, 3));
        orders->addOrder(new Advance(&alice, orderOntario, orderQuebec, 3));
        orders->addOrder(new Bomb(&alice, &manitoba));
        orders->addOrder(new Blockade(&alice, orderQuebec));
        orders->addOrder(new Airlift(&alice, orderOntario, orderQuebec, 3));
        orders->addOrder(new Negotiate(&alice, &carol));
        // Check that orders use Bob's territory copies and replace links to Alice.
        // Also check that orders with missing information stay invalid.
        orders->addOrder(new Airlift(&alice, &ontario, &quebec, 6));
        orders->addOrder(new Negotiate(&carol, &alice));
        orders->addOrder(new Negotiate(&alice, &alice));
        orders->addOrder(new Deploy());
        orders->addOrder(new Order());
        for (int index = 0; index < 6; ++index) {
            executeForPlayerCopy(orders->getOrder(index));
        }
        for (int index = 0; index < 11; ++index) {
            storedText.push_back(orderText(orders->getOrder(index)));
        }
        if (assignment) {
            bob = make_unique<Player>();
            bob->addTerritory(&manitoba);
            bob->issueOrder("Deploy");
            *bob = alice;
        } else {
            bob = make_unique<Player>(alice);
        }
        for (int index = 0; index < 11; ++index) {
            Order* original = orders->getOrder(index);
            Order* copied = bob->getOrdersList()->getOrder(index);
            checkPlayerCopy(original != copied && typeid(*original) == typeid(*copied),
                            scenario + " preserves separate order types at " + to_string(index));
            checkPlayerCopy(orderText(copied) == storedText[index], scenario + " preserves stored output");
        }
        checkPlayerCopy(!bob->getOrdersList()->getOrder(6)->validate(),
                        scenario + " uses owned Ontario's four armies instead of the map's ten");
        checkPlayerCopy(bob->getTerritoryList()[0] != alice.getTerritoryList()[0],
                        scenario + " creates separate territories");
        // Changing Alice's Ontario must not change Bob's Ontario.
        alice.getTerritoryList()[0]->setArmyCount(0);
    }
    bob->setName("Bob");
    const auto territories = bob->getTerritoryList();
    checkPlayerCopy(territories.size() == 2 && territories[0]->getOwner() == bob.get() &&
                    territories[1]->getOwner() == bob.get(), scenario + " installs Bob as owner");
    const auto& neighbours = territories[0]->getAdjacentTerritories();
    checkPlayerCopy(neighbours.size() == 2 && neighbours[0] == territories[1] &&
                    neighbours[1] == &manitoba &&
                    territories[1]->getAdjacentTerritories()[0] == territories[0],
                    scenario + " redirects internal adjacency and borrows external adjacency");
    checkPlayerCopy(territories[0]->getContinent() == &continent &&
                    bob->getCardHand()->getDeck() == &deck && bob->getCardHand()->getSize() == 1,
                    scenario + " copies cards while borrowing deck and continent");
    for (int index = 0; index < 6; ++index) {
        Order* order = bob->getOrdersList()->getOrder(index);
        checkPlayerCopy(order->validate(), scenario + " survives Alice for order " + to_string(index));
        executeForPlayerCopy(order);
        checkPlayerCopy(orderText(order).find("Effect: Simulated") != string::npos,
                        scenario + " executes copied order " + to_string(index));
    }
    Order* partnerOrder = bob->getOrdersList()->getOrder(7);
    executeForPlayerCopy(partnerOrder);
    checkPlayerCopy(orderText(partnerOrder).find("between Carol and Bob") != string::npos,
                    scenario + " redirects Alice as negotiation partner");
    checkPlayerCopy(!bob->getOrdersList()->getOrder(8)->validate() &&
                    !bob->getOrdersList()->getOrder(9)->validate() &&
                    !bob->getOrdersList()->getOrder(10)->validate(),
                    scenario + " preserves self-negotiation and missing context as invalid");
    // Carol and Manitoba are shared, so Bob's orders must see changes to them.
    manitoba.setOwner(nullptr);
    checkPlayerCopy(!bob->getOrdersList()->getOrder(2)->validate(), scenario + " keeps external target shared");
    carol.setName("Carol renamed");
    executeForPlayerCopy(bob->getOrdersList()->getOrder(5));
    checkPlayerCopy(orderText(bob->getOrdersList()->getOrder(5)).find("Carol renamed") != string::npos,
                    scenario + " keeps opponent shared");
    territories[0]->setArmyCount(0);
    executeForPlayerCopy(bob->getOrdersList()->getOrder(1));
    checkPlayerCopy(orderText(bob->getOrdersList()->getOrder(1)).find("Effect:") == string::npos,
                    scenario + " revalidates copied context and clears old effects");
    checkPlayerCopy(territories[1]->getArmyCount() == 0 && territories[1]->getOwner() == bob.get(),
                    scenario + " execution remains simulated");
    Player& self = *bob;
    *bob = self;
    checkPlayerCopy(bob->getTerritoryList()[0] == territories[0], scenario + " supports self-assignment");
    cout << "PASS: Player " << scenario << " survives source destruction with contextual orders\n";
}

// Make copying fail on purpose, then check that Bob still has his old data.
class UncopyableOrder : public Order {
public:
    Order* clone() const override;
};

Order* UncopyableOrder::clone() const {
    throw runtime_error("Deliberate order-copy failure");
}

void testPlayerAssignmentFailurePreservesDestination() {
    Player alice;
    alice.getOrdersList()->addOrder(new UncopyableOrder());
    Player bob;
    bob.setName("Bob");
    Territory ontario(1, "Ontario");
    bob.addTerritory(&ontario);
    bob.getOrdersList()->addOrder(new Deploy(&bob, bob.getTerritoryList()[0], 1));
    Territory* oldOntario = bob.getTerritoryList()[0];
    Order* oldOrder = bob.getOrdersList()->getOrder(0);
    bool failed = false;
    try {
        bob = alice;
    } catch (const runtime_error& error) {
        checkPlayerCopy(string(error.what()) == "Deliberate order-copy failure", "expected clone failure");
        failed = true;
    }
    checkPlayerCopy(failed && bob.getName() == "Bob" && bob.getTerritoryList()[0] == oldOntario &&
                    bob.getOrdersList()->getOrder(0) == oldOrder && oldOrder->validate(),
                    "failed assignment preserves Bob's existing state");
    cout << "PASS: Failed Player assignment preserves destination state\n";
}

void testEmptyPlayerCopiesAndIncompleteOrders() {
    Player alice;
    Player bob(alice);
    Territory ontario(1, "Ontario");
    bob.addTerritory(&ontario);
    bob = alice;
    checkPlayerCopy(bob.getTerritoryList().empty() && bob.getCardHand()->getSize() == 0,
                    "empty assignment replaces old territories");
    for (const string& type : {"Deploy", "Advance", "Bomb", "Blockade", "Airlift", "Negotiate"}) {
        alice.issueOrder(type);
    }
    Player copied(alice);
    bob = alice;
    for (int index = 0; index < 6; ++index) {
        checkPlayerCopy(!copied.getOrdersList()->getOrder(index)->validate() &&
                        !bob.getOrdersList()->getOrder(index)->validate(),
                        "both copy paths preserve missing context for order " + to_string(index));
    }
    cout << "PASS: Empty Player copies and incomplete orders remain valid objects\n";
}

} // namespace

bool testPlayer(const std::filesystem::path& mapFile) {
    testPlayerCopiesSurviveSource(false, false);
    testPlayerCopiesSurviveSource(true, false);
    testPlayerCopiesSurviveSource(false, true);
    testPlayerCopiesSurviveSource(true, true);
    testPlayerAssignmentFailurePreservesDestination();
    testEmptyPlayerCopiesAndIncompleteOrders();

    //=========Read the file=======
    MapLoader* loader = new MapLoader();
    Map* gameMap = nullptr;
    try {
        //use the nyc.map for testing
        gameMap = loader->loadMap(mapFile.string()).release();
    } catch (std::exception& error) {
        std::cout << "Could not load map: " << error.what() << std::endl;
        delete loader;
        return false;
    }
    
    //=====create players=====
    Player* player1= new Player();
    //give player a name
    player1->setName("Jane Doe");

    cout<<endl<<"==============================[TESTING: SET TERRITORY]================================================="<<endl;
    testSetTerritory(gameMap, player1);

    cout<<endl<<"==============================[TESTING: SET ORDER]================================================="<<endl;
    testingSetOrder(player1);

    cout<<endl<<"==============================[TESTING: SET CARD]================================================="<<endl;
    //create desk so we can use it for hands
    Deck* deck = new Deck();
    testSetCardHand(player1, deck);
    
    
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
    return true;
}

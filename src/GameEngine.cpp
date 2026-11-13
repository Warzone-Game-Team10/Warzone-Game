#include "GameEngine.h"
#include "CommandProcessing.h"
#include "Player.h"



// constructors -----------------------------
// first state is always start
GameEngine::GameEngine() : currentState("start"), _currentMap(nullptr){
    buildTransitions();
}

// copy constructor
GameEngine::GameEngine(const GameEngine& other)
    : Subject(other),currentState(other.currentState), transitions(other.transitions) {
}

// assignment operator
GameEngine& GameEngine::operator=(const GameEngine& other) {
    if (this != &other) {
        Subject::operator=(other);
        currentState = other.currentState;
        transitions = other.transitions;
    }
    return *this;
}

// destructor
GameEngine::~GameEngine() {

    delete _currentMap; // Clean up map memory if owned by engine
}

// getter
std::string GameEngine::getCurrentStateName() const {
    return currentState;
}

// game engine content --------------------------
// transition table : transitions[fromState][command] = toState (diagram)
void GameEngine::buildTransitions() {
    // startup phase
    transitions["start"]["loadmap"] = "map loaded";
    transitions["map loaded"]["loadmap"] = "map loaded"; //self loop
    transitions["map loaded"]["validatemap"] = "map validated";
    transitions["map validated"]["addplayer"] = "players added";
    transitions["players added"]["addplayer"] = "players added";
    transitions["players added"]["gamestart"] = "assign reinforcement";

    // play phase
    transitions["assign reinforcement"]["issueorder"] = "issue orders";
    transitions["issue orders"]["issueorder"] = "issue orders";
    transitions["issue orders"]["endissueorders"] = "execute orders";
    transitions["execute orders"]["execorder"] = "execute orders";
    transitions["execute orders"]["endexecorders"] = "assign reinforcement";
    transitions["execute orders"]["win"] = "win";
    transitions["win"]["replay"] = "start";
    transitions["win"]["quit"] = "end";
}

// applies a matching transition and returns false if none exists
bool GameEngine::processCommand(const std::string& command) {

    //look through entries
    std::map<std::string, std::map<std::string, std::string>>::const_iterator state = transitions.find(currentState); 

    if (state != transitions.end()) {
        std::map<std::string, std::string>::const_iterator edge = state->second.find(command);
        if (edge != state->second.end()) {
            currentState = edge->second;
            return true;
        }
    }

    return false;
}



// returns whether the engine has reached the end state
bool GameEngine::isFinished() const {
    return currentState == "end";
}

// stream insertion operator
std::ostream& operator<<(std::ostream& out, const GameEngine& e) {
    out << "GameEngine current state: " << e.currentState;
    return out;
}

// applies a command transition and stores its success or failure effect
bool GameEngine::processCommand(Command& command) {
    const std::string before = currentState;
    const std::string name = command.getName();
    const std::string argument = command.getArgument();
    if (!processCommand(name)) {
        command.saveEffect("Transition failed: command '" + name +
                           "' has no transition in state '" + before + "'.");
        return false;
    }
    command.saveEffect("Transition: " + name +
                       (argument.empty() ? "" : " [" + argument + "]") +
                       ": '" + before + "' -> '" + currentState + "'.");
    return true;
}

//Logging Observer part
string GameEngine::stringToLog( ) const{
    return "Game Engine: " + currentState;
}


//--------------------------------------- ASSIGNMENT #2 : PART 3 --------------------------------------------------------
/*
 * the reinforcement pool is the total number of army units given 
 * to a player at the start of their turn that are waiting to be deployed 
 * onto the map
 * 
 */
// =====================================   REINFORCEMENT PHASE  ==================================================
/**
 * reinforcementPhase() implementation
 * Calculates reinforcements based on territories and continent control
 */

void GameEngine::reinforcementPhase(std::vector<Player*>& players){

    if(players.empty())
    {
        return; // this acts as a guard clause, if there are no players, we stop here.  
    }

    
    std::cout << "\n=== REINFORCEMENT PHASE ===" << std::endl;

    for(Player* player : players )
    {
        if(player == nullptr)
        {
            continue; // (safety check) skip if this player pointer is null 
        }



        // 1. Base calculation: territories owned / 3 (integer division rounds down)
        int numTerritories = player->getTerritoryList().size();
        int baseArmies = numTerritories / 3;  // ( apparently c++ automatically rounds down... didnt know abt that LOL )



        // 2. Continent Control Bonus Calculation 
        int continentBonus = 0;
        if (_currentMap != nullptr)
        {


            const auto& continents = _currentMap->getContinents();
            const auto& allTerritories = _currentMap->getTerritories();


            // loop through every continent on the map 
            for(const auto& continentPtr : continents)
            {
                Continent* continent = continentPtr.get();
                if(!continent) continue;  // if continent pointer happens to be null, we move on to the next iteration 


                bool ownsAll = true;
                bool hasTerritoriesInContinent = false;  // to make sure a continent isnt accidentally empty or broken before we award bonus for it 

                // Check all map territories to see which ones belong to this continent
                /*
                    we have to look across all territories on the map to find the ones that
                     belong to the current continent we are testing.
                */
                for (const auto& territoryPtr : allTerritories) {
                    Territory* territory = territoryPtr.get();
                    if (territory && territory->getContinent() == continent) {
                        hasTerritoriesInContinent = true;


                        // If any territory in this continent is owned by someone else, they don't own it
                        /*
                            it basically asks : " Does this territory belong to someone else (or no one)? "
                        */
                        if (territory->getOwner() != player) {
                            ownsAll = false;
                            break;
                        }
                    }
                }

                
                

                // If the player owns every territory in this continent, add the bonus!
                if (hasTerritoriesInContinent && ownsAll) {
                    continentBonus += continent->getControlBonus();
                    std::cout << "  " << player->getName() << " controls continent "
                              << continent->getName() << " (+" << continent->getControlBonus() << " bonus)\n";

                }
            }
        }

        // 2. Total armies = base territories armies + continent bonuses
        int totalArmies = baseArmies + continentBonus;
            
  
        

                


        // Minimum of 3 units guaranteed
        if( totalArmies < 3 )
        {
            totalArmies = 3; // minimum rule which is " the minimal number of reinforcement armies per turn for any player is 3. "
        }

        player-> addReinforcementArmies(totalArmies);




        std::cout << player->getName() << " owns " << numTerritories 
                  << " territories (" << baseArmies << " base + " << continentBonus 
                  << " continent bonus) and receives " << totalArmies 
                  << " reinforcement armies. (Pool total: " 
                  << player->getReinforcementPool() << ")\n";
    

    }

    std::cout << "=== REINFORCEMENT PHASE COMPLETE ===\n" << std::endl;



}




// =====================================   ISSUE ORDER PHASE  ==================================================
/**
 * issueOrdersPhase() implementation
 * Calls Player::issueOrder() in round-robin fashion
 * 
 * The game engine goes around the table in a round-robin 
 * circle asking each player what they want to do.
 * 
 * Players look at their cards, check what to attack or defend, 
 * and push orders into their OrdersList using player->issueOrder()
*/

void GameEngine::issueOrdersPhase(std::vector<Player*>& players){

    if (players.empty()) {
        return; // Guard clause: if there are no players, stop here.
    }

    std::cout << "\n=== ISSUE ORDERS PHASE ===" << std::endl;


    int round = 1; 
    const int MAX_ROUNDS = 10; // ( safety limit for testing.. )


    // ROUND-ROBIN LOOP : keep going around the players until everyone finishes issuing orders
    while(round <= MAX_ROUNDS ){

        std::cout << "\n --- Order Issuing Round " << round << " ---" << std::endl ;

        for(Player* player : players){
            if(player == nullptr){
                continue;  // safety check 
            }


            // skip players who have no territories left (eliminated)
            if(player->getTerritoryList().empty()){
                continue; 
            }

            std::cout << "\n" << player->getName() << "'s turn to issue orders: " << std::endl;
            std::cout << " Reinforcement pool : " << player->getReinforcementPool() << " units" << std::endl; 

            
            // 1. if the player has armies in the reinforcement pool, issue a deploy order
            if(player->getReinforcementPool()>0){

                std::cout << " issuing a Deploy order... " << std::endl;
                player->issueOrder("Deploy");

            }
            // 2. otherwise, issue a tactical order (like advance)
            
            else {
                std::cout << " Issuing tactical order (advance)";
                player->issueOrder("Advance");

                /**
                * Once their reinforcement pool is empty (0), they aren't done for the turn! 
                * They still need to issue orders to move armies or attack enemies.
                */
            }
        }

        round++;


    }

    std::cout << "\n=== ISSUE ORDERS PHASE COMPLETE ===\n" << std::endl;




}



// =====================================   ORDERS EXECUTION PHASE  ================================================== 
/**
 * executeOrdersPhase() implementation
 * Executes orders from each player's order list in round-robin fashion.
 * All deploy orders are executed first, then remaining tactical orders.
 * 
 * Once everyone is completely done writing down their orders for the turn,
 * the game engine steps in to process them
 * 
 * The engine grabs the orders from everyone's lists and calls execute() on them.
 */

void GameEngine::executeOrdersPhase(std::vector<Player*>& players)
{
    if(players.empty()) {
        return; // if there are no players, we stop here. 
    }


    std::cout << "\n=== EXECUTE ORDERS PHASE ===" << std::endl;



    // ---------------------------------------------------------------------
    // first pass : execute all DEPLOY orders first across all players
    // ---------------------------------------------------------------------
    
    std::cout << "\n--- Executing Deploy Orders ---" << std::endl; 
    bool deployOrdersRemain = true; 

    while(deployOrdersRemain){
        deployOrdersRemain = false;  // Assume no deploy orders are left unless we find one

        for(Player* player : players){

            if(player == nullptr){
                continue; 
            }

            OrdersList* orderslist = player->getOrdersList();
            if(orderslist == nullptr){
                continue;
            }

            // we look through the players list to find the first deploy order
            // Note that since OrdersList doesnt have getSize(), we attempt to retrieve orders 
            // starting at index 0 until getOrder throws or returns nullptr.
            for(int i=0; ;  ++i){
                try{
                    Order* order = orderslist->getOrder(i);
                    if(order==nullptr){
                        break;
                    }

                    //check if this order is a deploy order... 
                    Deploy* deployOrder = dynamic_cast<Deploy*>(order);
                    if (deployOrder != nullptr) {

                        std::cout << "Executing " << player->getName() << "'s Deploy order..." << std::endl;
                        
                        // Execute the order
                        order->execute();

                        // Remove the executed Deploy order from the list
                        orderslist->remove(i);

                        deployOrdersRemain = true; // We executed one, so re-check in the next pass
                        break; // Move to the next player in round-robin fashion
                    }

                }
                catch(...){
                    // Out of bounds - no more orders in this player's list
                    break;
                }
            }

        }
    }



    // ---------------------------------------------------------------------
    // second pass: we execute all REMAINING orders in a round robin fashion
    // ---------------------------------------------------------------------
    std::cout << "\n--- Executing Remaining Tactical orders ---" << std::endl;
    bool remainingOrdersExist = true;
    
    while(remainingOrdersExist){
        remainingOrdersExist = false; // Assume everyone is out of orders

        for(Player* player : players){
            if(player == nullptr){
                continue; 
            }

            OrdersList* orderslist = player->getOrdersList();
            if(orderslist == nullptr){
                continue; 
            }

            // try to grab the top order (index 0)
            try {
                Order* topOrder = orderslist->getOrder(0);
                if (topOrder != nullptr) {
                    std::cout << "Executing " << player->getName() << "'s order..." << std::endl;
                    
                    // Execute top order
                    topOrder->execute();

                    // Remove top order
                    orderslist->remove(0);

                    remainingOrdersExist = true; // Still processing orders, keep loop active
                }
            } catch (...) {
                // Player's list is empty, move on
                continue;
            }
        }
    }






}


// =====================================   MAIN GAME LOOP  ================================================== 
/**
 * mainGameLoop() implementation
 * Runs the core game loop: Reinforcement -> Issue Orders -> Execute Orders.
 * Checks for player elimination and victory conditions after each turn cycle.
 */

 void GameEngine::mainGameLoop(std::vector<Player*>& players){

    if (players.empty()) {
        std::cout << "Error: No players in the game to start the main loop." << std::endl;
        return;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "     MAIN GAME LOOP STARTED" << std::endl;
    std::cout << "========================================\n"
              << std::endl;


    bool gameWon = false;
    int maxTurns = 50; // Safety limit for testing drivers
    int currentTurn = 1;   


    while (!gameWon && currentTurn <= maxTurns){
        std::cout << "\n#############################################" << std::endl;
        std::cout << "              --- TURN " << currentTurn << " ---" << std::endl;
        std::cout << "#############################################\n" << std::endl;
        

        // 1. CLEAN UP ELIMINATED PLAYERS AT THE START OF THE TURN
        for (auto it = players.begin(); it != players.end(); ) {

            Player* player = *it;
            
            if (player != nullptr && player->getTerritoryList().empty()) {
                std::cout << "\n*** Player " << player->getName() << " has been eliminated! ***\n" << std::endl;
                it = players.erase(it);

            } else {
                ++it;
            }
        }


        // 2. CHECK WIN CONDITION AT THE START
        if (players.size() == 1) {
            std::cout << "\n=============================================" << std::endl;
            std::cout << "  VICTORY! Winner is " << players[0]->getName() << "!" << std::endl;
            std::cout << "=============================================" << std::endl;
            gameWon = true;
            break;
        }


        // 3. RUN THE THREE PHASES
        reinforcementPhase(players);
        issueOrdersPhase(players);
        executeOrdersPhase(players);

        currentTurn++;
        
    }

    if (currentTurn > maxTurns) {
        std::cout << "Main game loop reached maximum test turns limit." << std::endl;
    }
   
   
 }



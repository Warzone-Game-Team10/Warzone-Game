#include "Orders.h"
#include "Player.h"
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <sstream>//used for std::ostringstream text; in stringToLog function

//==========================================================================================
//                  1. PARENT CLASS ( ORDER ) IMPLEMENTATION 
//==========================================================================================


// ----------  DEFAULT CONSTRUCTOR ------------
Order::Order() : description(nullptr), effect(nullptr), executed(false) {
    // Keep temporary ownership until both string allocations succeed.
    auto text = std::make_unique<std::string>("Generic Warzone Order");
    effect = new std::string();
    description = text.release();
}

// ---------- COPY CONSTRUCTOR (DEEP COPY) ----------
// Copies own their description and effect strings, but subclasses still borrow players and territories.
Order::Order(const Order& other) :  Subject(other),description(nullptr), effect(nullptr), executed(other.executed) {
    std::unique_ptr<std::string> text(other.description ? new std::string(*other.description) : nullptr);
    effect = new std::string(*other.effect);
    description = text.release();
}

// ---------- DESTRUCTOR ----------
Order::~Order() {
    delete description;
    delete effect;
}

// ---------- ASSIGNMENT OPERATOR ---------- 
Order& Order::operator=(const Order& other) {
    if (this != &other) {
        // Finish allocating both strings before replacing any existing state.
        Subject::operator=(other);
        Order copy(other);
        std::swap(description, copy.description);
        std::swap(effect, copy.effect);
        std::swap(executed, copy.executed);
    }
    return *this;
}

// Copy through the public interface without losing the dynamic order type.
Order* Order::clone() const {
    return new Order(*this);
}

Deploy* Deploy::clone() const {
    return new Deploy(*this);
}

Advance* Advance::clone() const {
    return new Advance(*this);
}

Bomb* Bomb::clone() const {
    return new Bomb(*this);
}

Blockade* Blockade::clone() const {
    return new Blockade(*this);
}

Airlift* Airlift::clone() const {
    return new Airlift(*this);
}

Negotiate* Negotiate::clone() const {
    return new Negotiate(*this);
}

// Find the copied territory by ID. For example, a link to the map's Ontario
// becomes a link to the copied player's Ontario when their IDs match.
namespace {
Territory* copiedTerritory(Territory* territory,
                           const std::unordered_map<int, Territory*>& territories) {
    if (territory == nullptr || territories.empty()) {
        return territory;
    }
    const auto found = territories.find(territory->getId());
    return found == territories.end() ? territory : found->second;
}
}

void Order::rebindContext(const Player*, Player*,
                          const std::unordered_map<int, Territory*>&) {}

void Deploy::rebindContext(const Player* original, Player* destination,
                            const std::unordered_map<int, Territory*>& territories) {
    if (issuer == original) {
        issuer = destination;
    }
    target = copiedTerritory(target, territories);
}

void Advance::rebindContext(const Player* original, Player* destination,
                            const std::unordered_map<int, Territory*>& territories) {
    if (issuer == original) {
        issuer = destination;
    }
    source = copiedTerritory(source, territories);
    target = copiedTerritory(target, territories);
}

void Bomb::rebindContext(const Player* original, Player* destination,
                            const std::unordered_map<int, Territory*>& territories) {
    if (issuer == original) {
        issuer = destination;
    }
    target = copiedTerritory(target, territories);
}

void Blockade::rebindContext(const Player* original, Player* destination,
                            const std::unordered_map<int, Territory*>& territories) {
    if (issuer == original) {
        issuer = destination;
    }
    target = copiedTerritory(target, territories);
}

void Airlift::rebindContext(const Player* original, Player* destination,
                            const std::unordered_map<int, Territory*>& territories) {
    if (issuer == original) {
        issuer = destination;
    }
    source = copiedTerritory(source, territories);
    target = copiedTerritory(target, territories);
}

void Negotiate::rebindContext(const Player* original, Player* destination,
                            const std::unordered_map<int, Territory*>&) {
    if (issuer == original) {
        issuer = destination;
    }
    if (partner == original) {
        partner = destination;
    }
}

void OrdersList::rebindContext(const Player* original, Player* destination,
                               const std::unordered_map<int, Territory*>& territories) {
    for (Order* order : list) {
        order->rebindContext(original, destination, territories);
    }
}


// ------------------- VALIDATE +  EXECUTE METHODS  ------------------- 

//VALIDATE METHOD : 

bool Order::validate(){
    // A generic order has no specific action whose requirements can be checked.
    return false;

}



//EXECUTE METHOD : 

bool Order::validateExecution() {
    executed = false;
    effect->clear();
    if (!validate()) {
        std::cout << "Order execution failed: invalid order (" << *description << ")." << std::endl;
        return false;
    }
    return true;
}

void Order::recordEffect(const std::string& action) {
    *effect = "Simulated " + action + " (game state unchanged).";
    executed = true;
    std::cout << "Executing demonstration: " << *effect << std::endl;
}

void Order::execute() {
    if (validateExecution()) {
        recordEffect("execution of " + *description);
    }
    notify(this);// notify observer
}

//=============================================================================================
//                  2.  CHILD CLASSES IMPLEMENTATION 
//=============================================================================================


// --------------- DEPLOY ORDER -----------------

// Default constructor
// Keep existing callers working; an order without context can be queued but is invalid.
Deploy::Deploy() : Deploy(nullptr, nullptr, 0) {}

// Parameterized constructor
Deploy::Deploy(Player* issuer, Territory* target, int armyCount)
    : Order(), issuer(issuer), target(target), armyCount(armyCount) {
    // We reuse the parent class constructor to set up a specific description
    *description = "Deploy Order";
    if (target != nullptr) {
        *description = "Deploy " + std::to_string(armyCount) + " armies to " + target->getName() + ".";
    }
}


// Copy constructor
// Deep-copy the description, but keep references to the same player and target.
Deploy::Deploy(const Deploy& other)
    : Order(other), issuer(other.issuer), target(other.target), armyCount(other.armyCount) {}


// Destructor 
Deploy::~Deploy() {
    // the second we get to the end of the closing brace " } " of the child destructor, 
    // C++ automatically jumps up and fires the parent destructor Order::~Order() on its own.
}


// Assignment Operator 
Deploy& Deploy::operator=(const Deploy& other){

    // 1. run the safety self-assignment check first 
    if(this != &other){
        // We call the parent's assignment operator to safely wipe out our old 
        // string allocation and deep-copy other's string allocation.
        Order::operator=(other);
        issuer = other.issuer;
        target = other.target;
        armyCount = other.armyCount;
    }
    return *this;
} 


// Validate
bool Deploy::validate() {
    // Use Player's collection-based ownership rules.
    return issuer != nullptr && target != nullptr && armyCount > 0 && issuer->ownsTerritory(target);
}


// Execute 
void Deploy::execute() {
    if (validateExecution()) {
        recordEffect("deployment of " + std::to_string(armyCount) + " armies to " + target->getName());
    }
    
    notify(this);// notify observer

}







// --------------- ADVANCE ORDER --------------- 

// Default constructor: incomplete orders remain queueable.
Advance::Advance() : Advance(nullptr, nullptr, nullptr, 0) {}

// Parameterized constructor
Advance::Advance(Player* issuer, Territory* source, Territory* target, int armyCount)
    : Order(), issuer(issuer), source(source), target(target), armyCount(armyCount) {
    *description = "Advance Order";
    if (source != nullptr && target != nullptr) {
        *description = "Advance " + std::to_string(armyCount) + " armies from " + source->getName() + " to " + target->getName() + ".";
    }
}


// Copy constructor: copy the description independently while preserving borrowed player and territory links.
Advance::Advance(const Advance& other)
    : Order(other), issuer(other.issuer), source(other.source), target(other.target), armyCount(other.armyCount) {}


// Destructor
Advance::~Advance() {}


// Assignment Operator
Advance& Advance::operator=(const Advance& other) {
    if (this != &other) {
        Order::operator=(other);
        issuer = other.issuer;
        source = other.source;
        target = other.target;
        armyCount = other.armyCount;
    }
    return *this;
}


// Validation Method
bool Advance::validate() {
    if (issuer == nullptr || source == nullptr || target == nullptr ||
        !issuer->ownsTerritory(source) || source->getId() == target->getId() ||
        armyCount <= 0 || armyCount > source->getArmyCount()) {
        return false;
    }
    // Player clones territories, so compare IDs
    for (Territory* neighbor : source->getAdjacentTerritories()) {
        if (neighbor != nullptr && neighbor->getId() == target->getId()) {
            return true;
        }
    }
    return false;
}


// Execution Method
void Advance::execute() {
    if (validateExecution()) {
        recordEffect("advance of " + std::to_string(armyCount) + " armies from " + source->getName() + " to " + target->getName());
    }
    notify(this);// notify observer

}







// --------------- BOMB ORDER --------------- 

// Default constructor: orders remain queueable, but lack the context needed for validation.
Bomb::Bomb() : Bomb(nullptr, nullptr) {}

// Parameterized constructor
Bomb::Bomb(Player* issuer, Territory* target)
    : Order(), issuer(issuer), target(target) {
    *description = "Bomb Order";
    if (target != nullptr) {
        *description = "Bomb " + target->getName() + ".";
    }
}



// Copy constructor: copy the description independently while preserving borrowed player and territory links.
Bomb::Bomb(const Bomb& other)
    : Order(other), issuer(other.issuer), target(other.target) {}



// Destructor
Bomb::~Bomb() {
    // Handled safely by automatic parent destructor chaining
}

// Assignment Operator
Bomb& Bomb::operator=(const Bomb& other) {
    if (this != &other) {
        Order::operator=(other);
        issuer = other.issuer;
        target = other.target;
    }
    return *this;
}



// Validation Method
bool Bomb::validate() {
    if (issuer == nullptr || target == nullptr || target->getOwner() == nullptr ||
        target->getOwner() == issuer || issuer->ownsTerritory(target)) {
        return false;
    }
    // Any owned territory may provide adjacency; IDs account for Player's territory copies.
    for (Territory* territory : issuer->getTerritoryList()) {
        for (Territory* neighbor : territory->getAdjacentTerritories()) {
            if (neighbor != nullptr && neighbor->getId() == target->getId()) {
                return true;
            }
        }
    }
    return false;
}



// Execution Method
void Bomb::execute() {
    if (validateExecution()) {
        recordEffect("bombing of " + target->getName());
    }

    notify(this);// notify observer

}







// --------------- BLOCKADE ORDER --------------- 

// Default constructor
// Keep existing callers working; an order without context can be queued but is invalid.
Blockade::Blockade() : Blockade(nullptr, nullptr) {}

// Parameterized constructor
Blockade::Blockade(Player* issuer, Territory* target)
    : Order(), issuer(issuer), target(target) {
    // we dereference our inherited pointer to assign our unique child text.
    *description = "Blockade Order";
    if (target != nullptr) {
        *description = "Blockade " + target->getName() + ".";
    }
}


// Copy constructor
// Deep-copy the description, but keep references to the same player and target.
Blockade::Blockade(const Blockade& other)
    : Order(other), issuer(other.issuer), target(other.target) {}


// destructor
Blockade::~Blockade() {
    
}


// assignment Operator
Blockade& Blockade::operator=(const Blockade& other) {
    // 1. Self-assignment safety check
    if (this != &other) {
        // 2. we reuse the parent class assignment logic to safely wipe out
        //    our current heap text and perform a clean deep-copy of 'other'.
        Order::operator=(other);
        issuer = other.issuer;
        target = other.target;
    }
    return *this;
}


// validation Method
bool Blockade::validate() {
    // Use the same collection-based ownership rules as Player.
    return issuer != nullptr && target != nullptr && issuer->ownsTerritory(target);
}


// Execution Method
void Blockade::execute() {
    if (validateExecution()) {
        recordEffect("blockade of " + target->getName());
    }
    
    notify(this);// notify observer

}





// ---------------  AIRLIFT ORDER --------------- 

// Default constructor: orders remain queueable, but lack the context needed for validation.
Airlift::Airlift() : Airlift(nullptr, nullptr, nullptr, 0) {}

// Parameterized constructor
Airlift::Airlift(Player* issuer, Territory* source, Territory* target, int armyCount)
    : Order(), issuer(issuer), source(source), target(target), armyCount(armyCount) {
    // We dereference our inherited pointer to assign our unique child text.
    *description = "Airlift Order";
    if (source != nullptr && target != nullptr) {
        *description = "Airlift " + std::to_string(armyCount) + " armies from " + source->getName() + " to " + target->getName() + ".";
    }
}


// Copy constructor: copy the description independently while preserving borrowed player and territory links.
Airlift::Airlift(const Airlift& other)
    : Order(other), issuer(other.issuer), source(other.source), target(other.target), armyCount(other.armyCount) {}


// Destructor
Airlift::~Airlift() {
}


// Assignment Operator
Airlift& Airlift::operator=(const Airlift& other) {
    if (this != &other) {
        Order::operator=(other);
        issuer = other.issuer;
        source = other.source;
        target = other.target;
        armyCount = other.armyCount;
    }
    return *this;
}


// Validation Method
bool Airlift::validate() {
    return issuer != nullptr && source != nullptr && target != nullptr &&
           issuer->ownsTerritory(source) && source->getId() != target->getId() &&
           armyCount > 0 && armyCount <= source->getArmyCount();
}


// Execution Method
void Airlift::execute() {
    if (validateExecution()) {
        recordEffect("airlift of " + std::to_string(armyCount) + " armies from " + source->getName() + " to " + target->getName());
    }
    
    notify(this);// notify observer

}







// --------------- NEGOTIATE ORDER --------------- 


// Default constructor: orders remain queueable, but lack the context needed for validation.
Negotiate::Negotiate() : Negotiate(nullptr, nullptr) {}

// Parameterized constructor
Negotiate::Negotiate(Player* issuer, Player* partner)
    : Order(), issuer(issuer), partner(partner) {
    // We dereference our inherited pointer to assign our unique child text.
    *description = "Negotiate Order";
    if (partner != nullptr) {
        *description = "Negotiate with " + partner->getName() + ".";
    }
}

// Copy constructor: copy the description independently while preserving borrowed player links.
Negotiate::Negotiate(const Negotiate& other)
    : Order(other), issuer(other.issuer), partner(other.partner) {}

// Destructor
Negotiate::~Negotiate() {
    // Stays empty! Automatic destructor chaining handles cleanup safely.
}

// Assignment Operator
Negotiate& Negotiate::operator=(const Negotiate& other) {
    if (this != &other) {
        Order::operator=(other);
        issuer = other.issuer;
        partner = other.partner;
    }
    return *this;
}

// Validation Method
bool Negotiate::validate() {
    return issuer != nullptr && partner != nullptr && issuer != partner;
}

// Execution Method
void Negotiate::execute() {
    if (validateExecution()) {
        recordEffect("negotiation between " + issuer->getName() + " and " + partner->getName());
    }
    
    notify(this);// notify observer

}










// =============================================================================================
//                   3. ORDERSLIST CLASS IMPLEMENTATION
// =============================================================================================


// default constructor 
OrdersList::OrdersList(){

}


// copy constructor ( deep copy!! )
OrdersList::OrdersList( const OrdersList& other) : Subject(other){
    try {
        for (const Order* order : other.list) {
            // Keep ownership until insertion succeeds, including allocation failures.
            std::unique_ptr<Order> copy(order->clone());
            list.push_back(nullptr);
            list.back() = copy.release();
        }
    } catch (...) {
        // A failed constructor does not run the OrdersList destructor.
        for (Order* order : list) {
            delete order;
        }
        throw;
    }
}


// destructor ( prevent memory leak by releasing resources )
OrdersList::~OrdersList() {
    // we loop through the list and delete every single remaining order thats on the heap
    for (size_t i = 0; i < list.size(); ++i) {
        if (list[i] != nullptr) {
            delete list[i];      // Blows up the object on the heap
            list[i] = nullptr;   // this is to avoid dangling pointers... 
        }
    }
    list.clear(); // Empties out the pointer boxes inside the vector array ( makes ur array size = 0 )
}


// assignment operator 
OrdersList& OrdersList::operator=(const OrdersList& other) {
    if (this != &other) {
        Subject::operator=(other);
        // Build the complete copy before replacing the destination's orders.
        OrdersList replacement(other);
        list.swap(replacement.list);
    }
    return *this;
}



// ADDING AN ORDER  
void OrdersList::addOrder(Order* newOrder) {
    if (newOrder != nullptr) {
        list.push_back(newOrder); // this puts the order pointer onto the back of the queue
    }
    notify(this);// notify observer
}


// ACCESSING AN ORDER
Order* OrdersList::getOrder(int index) {
    return list.at(index);
}


// REMOVING AN ORDER
void OrdersList::remove(int index) {
    // 1. Safety Guard: Check if the index is outside our list boundaries
    if (index < 0 || index >= static_cast<int>(list.size())) {
        std::cout << "Error: Index out of bounds! Cannot remove order." << std::endl;
        return; // Terminate early so the program doesn't perform unsafe lookups
    }

    // 2. Erase the underlying object data on the heap to prevent memory leaks
    Order* orderToDelete = list[index];
    if (orderToDelete != nullptr) {
        delete orderToDelete;
        orderToDelete = nullptr;
    }

    // 3. Remove the tracking address slot from the vector array layout entirely
    list.erase(list.begin() + index);
}



// MOVING AN ORDER
void OrdersList::move(int oldIndex, int newIndex) {
    int listSize = static_cast<int>(list.size());

    // 1. Safety Guard: Ensure both positions fit cleanly inside the collection
    if (oldIndex < 0 || oldIndex >= listSize || newIndex < 0 || newIndex >= listSize) {
        std::cout << "Error: Index out of bounds! Cannot shuffle order positions." << std::endl;
        return;
    }

    // 2. Efficiency Check: If indices are identical, no movement is needed
    if (oldIndex == newIndex) {
        return;
    }

    // 3. Temporarily isolate the tracking address pointer so it isn't lost
    Order* targetOrder = list[oldIndex];

    // 4. Erase the pointer tracker layout from its original slot position
    list.erase(list.begin() + oldIndex);

    // 5. Insert the pointer address safely back into its new priority slot position
    list.insert(list.begin() + newIndex, targetOrder);

}





// ==============================================================================================
//           4.  GLOBAL STREAM INSERTION OPERATORS IMPLEMENTATION
// ==============================================================================================

// Overloading << for the parent Order class and all its subclasses
std::ostream& operator<<(std::ostream& outs, const Order& order){
    // We dereference the internal description pointer to print the raw text words
    outs << "Order Description: " << *(order.description);
    if (order.executed) {
        outs << " Effect: " << *order.effect;
    }
    return outs;
}

// Overloading << for the OrdersList manager container class
std::ostream& operator<<(std::ostream& outs, const OrdersList& list) {
    outs << "=== Player Orders List ===\n";
    
    // Safety check: if our list vector is empty, tell the console
    if (list.list.empty()) {
        outs << "(No active orders in queue)\n";
    } else {
        // Loop through our vector of pointers and print each element line-by-line
        int count = 1;
        for (Order* ord : list.list) {
            if (ord != nullptr) {
                // The asterisk dereferences the pointer 'ord' so it fires the operator above!
                outs << "  " << count << ". " << *ord << "\n";
                count++;
            }
        }
    }
    outs << "==========================";
    return outs;
}



//Logging Observer part

// text logged when an order is added: only the newest order
std::string OrdersList::stringToLog() const {
    if (list.empty() || list.back() == nullptr) return "Order Issued: (none)";
    std::ostringstream text;
    text << "AddOrder(): " << *list.back();
    return text.str();
}

// text logged when an order is executed
std::string Order::stringToLog() const {
    return "Order Executed: " + *effect;
}







/*. ==================== ADDITIONAL NOTES ===========================

    The Stack: A temporary, fast memory zone where standard variables are stored. 
    Objects created here are deleted automatically by the computer when a function ends.


    The Heap: A massive pool of permanent system memory. Objects created here using the 
    keyword new stay in your RAM forever until you manually delete them.


    Memory Leak: This happens when you lose or delete a pointer variable on the Stack, 
    but leave the allocated data sitting on the Heap. The data becomes trapped in your RAM chips forever.


    Dangling Pointer: A pointer variable that still holds an address number, but the data at 
    that address has already been deleted by another part of the program.


*/

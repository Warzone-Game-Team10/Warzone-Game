#include "Orders.h"
#include <iostream>
#include <string> 

//==========================================================================================
//                  1. PARENT CLASS ( ORDER ) IMPLEMENTATION 
//==========================================================================================


// ----------  DEFAULT CONSTRUCTOR ------------
Order::Order(){

    description = new std::string("Generic Warzone Order");

}


// --------- COPY CONSTRUCTOR (DEEP COPY) ---------
Order::Order(const Order& other){ 
    // 

    // 1. check if original object has a valid description block 
    if(other.description != nullptr){

        // 2. follow (other)'s arrow ( where its pointing at ), read the text n allocate a 
        //    a brand new block on the heap, n store the copied text inside it.
        description = new std::string(*(other.description));
    }
    else{
        description = nullptr;
    }
}


// ------------------- DESTRUCTOR (prevents memory leaks) ------------------- 
Order::~Order(){

    // 1. we check if our pointer variable is currently holding a valid memory address .. 
    if(description != nullptr){

        delete description;         // follows where the pointer is pointing at, locates the string box on the heap n manually deletes it 
                                    // frees the dynamic memory allocated for the string data on the heap
        
        description = nullptr;      /*
                                     Wipes out the old dead address number inside our pointer variable and 
                                     sets it to nullptr (sets pointer to null)
                                     This completely prevents a Dangling Pointer error by ensuring
                                     the pointer doesn't accidentally point to empty, raw computer memory.
                                    */ 
        
    }
}


// ------------------- ASSIGNMENT OPERATOR  ------------------- 

Order& Order::operator=(const Order& other) {
    // 1. Self-assignment check: make sure we aren't doing "order1 = order1;
    if(this != &other){

        // 2. clean up the old existing heap memory to prevent leaks 
        if(description != nullptr){
            delete description;
        }


        // 3. perform a deep copy of the text data 
        if(other.description != nullptr){
            description = new std::string(*(other.description));
        } else {
            description = nullptr;
        }

    }

    return *this;
}




// ------------------- VALIDATE +  EXECUTE METHODS  ------------------- 

//VALIDATE METHOD : 

bool Order::validate(){
    //since its a generic order, we'll just return true...
    std::cout << " Validating generic order " << std::endl;
    return true;

}



//EXECUTE METHOD : 

void Order::execute(){
    // we must run validate() first!
    if(validate()){
        std::cout << " Executing order : " << *description << std::endl;
    } else {
        std::cout << " Order execution failed : invalid order. " << std::endl;

    }
}










//=============================================================================================
//                  2.  CHILD CLASSES IMPLEMENTATION 
//=============================================================================================


// --------------- DEPLOY ORDER -----------------

// default constructor 
Deploy::Deploy() : Order() {
    // We reuse the parent class constructor to set up a specific description
    *description = "Deploy Order";
}


// copy constructor (deep copy)
Deploy::Deploy( const Deploy& other): Order(other){
    // Since Deploy doesn't have any extra local pointer fields of its own yet, we can safely leave it as empty
    // and let the parent class handle the deep copying
}


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
    }
    return *this;
} 


// Validate
bool Deploy::validate() {
    std::cout << "Validating Deploy order logic..." << std::endl;
    return true; 
}


// Execute 
void Deploy::execute() {
    if (validate()) {
        std::cout << "Executing action: Troops deployed successfully!" << std::endl;
    }
}







// --------------- ADVANCE ORDER --------------- 

// Default Constructor
Advance::Advance() : Order() {
    *description = "Advance Order"; 
}


// Copy Constructor
Advance::Advance(const Advance& other) : Order(other) {}


// Destructor
Advance::~Advance() {}


// Assignment Operator
Advance& Advance::operator=(const Advance& other) {
    if (this != &other) {
        Order::operator=(other);
    }
    return *this;
}


// Validation Method
bool Advance::validate() {
    std::cout << "Validating Advance order logic..." << std::endl;
    return true; 
}


// Execution Method
void Advance::execute() {
    if (validate()) {
        std::cout << "Executing Action: Troops advanced across territories!" << std::endl;
    }
}







// --------------- BOMB ORDER --------------- 

// Default Constructor
Bomb::Bomb() : Order() {
    *description = "Bomb Order"; 
}



// Copy Constructor
Bomb::Bomb(const Bomb& other) : Order(other) {
    // Handled safely by the parent deep copy setup
}



// Destructor
Bomb::~Bomb() {
    // Handled safely by automatic parent destructor chaining
}

// Assignment Operator
Bomb& Bomb::operator=(const Bomb& other) {
    if (this != &other) {
        Order::operator=(other); // Invokes parent scope assignment safety
    }
    return *this;
}



// Validation Method
bool Bomb::validate() {
    std::cout << "Validating Bomb order logic..." << std::endl;
    return true; 
}



// Execution Method
void Bomb::execute() {
    if (validate()) {
        std::cout << "Executing Action: Target territory bombed! Enemy forces halved!" << std::endl;
    }
}







// --------------- BLOCKADE ORDER --------------- 

// Default Constructor
Blockade::Blockade() : Order() {
    // we dereference our inherited pointer to assign our unique child text.
    *description = "Blockade Order"; 
}


// Copy Constructor
Blockade::Blockade(const Blockade& other) : Order(other) {
    // By passing 'other' up to : Order(other), the parent class takes care 
    // of allocating a brand-new slot on the heap and duplicating the text safely.
}


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
    }
    return *this;
}


// validation Method
bool Blockade::validate() {
    std::cout << "Validating Blockade order logic..." << std::endl;
    return true; 
}


// Execution Method
void Blockade::execute() {
    // execute() must call validate() first
    if (validate()) {
        std::cout << "Executing Action: Territory blockaded! Friendly troops doubled, territory turned neutral!" << std::endl;
    }
}





// ---------------  AIRLIFT ORDER --------------- 

// Default Constructor
Airlift::Airlift() : Order() {
    // We dereference our inherited pointer to assign our unique child text.
    *description = "Airlift Order"; 
}


// Copy Constructor
Airlift::Airlift(const Airlift& other) : Order(other) {
}


// Destructor
Airlift::~Airlift() {
}


// Assignment Operator
Airlift& Airlift::operator=(const Airlift& other) {
    if (this != &other) {
        Order::operator=(other); 
    }
    return *this;
}


// Validation Method
bool Airlift::validate() {
    std::cout << "Validating Airlift order logic..." << std::endl;
    return true; 
}


// Execution Method
void Airlift::execute() {
    if (validate()) {
        std::cout << "Executing Action: Troops airlifted to a distant territory!" << std::endl;
    }
}







// --------------- NEGOTIATE ORDER --------------- 


// Default Constructor
Negotiate::Negotiate() : Order() {
    // We dereference our inherited pointer to assign our unique child text.
    *description = "Negotiate Order"; 
}

// Copy Constructor
Negotiate::Negotiate(const Negotiate& other) : Order(other) {
    // Handled safely by the parent deep copy setup
}

// Destructor
Negotiate::~Negotiate() {
    // Stays empty! Automatic destructor chaining handles cleanup safely.
}

// Assignment Operator
Negotiate& Negotiate::operator=(const Negotiate& other) {
    if (this != &other) {
        Order::operator=(other); // Invokes parent scope assignment safety
    }
    return *this;
}

// Validation Method
bool Negotiate::validate() {
    std::cout << "Validating Negotiate order logic..." << std::endl;
    return true; // Baseline placeholder for Assignment 1 requirements
}

// Execution Method
void Negotiate::execute() {
    if (validate()) {
        std::cout << "Executing Action: Peace agreement signed! Attacks prevented between players for this turn!" << std::endl;
    }
}










// =============================================================================================
//                   3. ORDERSLIST CLASS IMPLEMENTATION
// =============================================================================================


// default constructor 
OrdersList::OrdersList(){

}


// copy constructor ( deep copy!! )
OrdersList::OrdersList( const OrdersList& other){
    if(this != &other){
    // Loop through the other list, find every order pointer, 
    // duplicate its text deeply, and add it to our new vector.
    for (size_t i = 0; i < other.list.size(); ++i) {
    if (other.list[i] != nullptr) {
        // This copies the base instance reference safely without slicing object boundaries
        list.push_back(new Order(*other.list[i])); 
    }
}

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
        // 1. clear out our own existing memory so we dont leak anything
        for (size_t i = 0; i < list.size(); ++i) {
            if (list[i] != nullptr) {
                delete list[i];
            }
        }
        list.clear();

        // 2. perform a deep copy from the 'other' list
        for (size_t i = 0; i < other.list.size(); ++i) {
            if (other.list[i] != nullptr) {
                list.push_back(new Order(*other.list[i]));
            }
        }
    }
    return *this;
}



// ADDING AN ORDER  
void OrdersList::addOrder(Order* newOrder) {
    if (newOrder != nullptr) {
        list.push_back(newOrder); // this puts the order pointer onto the back of the queue
    }
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
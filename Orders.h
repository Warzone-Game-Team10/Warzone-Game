//Preprocessor directives: These are guards that prevent your file from being read 
//multiple times by the compiler, which would cause an error.

#ifndef ORDERS_H
#define ORDERS_H

#include <iostream>  // will allow us to print text to the console 
#include <vector> 
#include <string>


//============================= Blueprint for the (parent) Order class =============================

class Order{

public: 

    // CONSTRUCTORS 
    Order();                    // default constructor 
    Order(const Order& other);  // copy constructor 
    virtual ~Order();           // destructor 


    // ASSIGNMENT OPERATOR 
    Order& operator=(const Order& other);   // w/o this C++ defaults to a shallow copy when we assign smth like order1=order2 

    virtual bool validate();       // method which verifies if order is illegal 
    virtual void execute();        // method which executes the game action 


private:

    // a pointer to a string that would hold the order's description 
    std::string* description; 



};

//============================= 6 SUBLCASSES (DIFFERENT ORDERS)  =============================


// -----  1. DEPLOY ORDER -----------

class Deploy : public Order {
public: 

    Deploy();                       // Default Constructor 
    Deploy(const Deploy& other);    // Copy constructor 
    ~Deploy() override;             // Destructor 

    Deploy& operator=(const Deploy& other);  // assignment operator 

    bool validate() override;       
    void execute() override; 
};



// -----  2. ADVANCE ORDER -----------

class Advance : public Order {
public: 

    Advance();
    Advance(const Advance& other);
    ~Advance() override; 

    Advance& operator=(const Advance& other);

    bool validate() override; 
    void execute() override; 
};



// -----  3. BOMB ORDER -----------

class Bomb : public Order {
public: 

    Bomb();
    Bomb(const Bomb& other);
    ~Bomb() override; 

    Bomb& operator=(const Bomb& other);

    bool validate() override; 
    void execute() override; 

};



// -----  4. BLOCKADE ORDER -----------

class Blockade : public Order {
public: 

    Blockade();
    Blockade(const Blockade& other);
    ~Blockade() override;

    Blockade& operator=( const Blockade& other);

    bool validate() override; 
    void execute() override; 
};



// -----  5. AIRLIFT ORDER -----------

class Airlift : public Order {
public: 
    Airlift();
    Airlift(const Airlift& other);
    ~Airlift() override; 

    Airlift& operator=(const Airlift& other);

    bool validate() override;
    void execute() override;
};



// -----  6. NEGOTIATE ORDER -----------

class Negotiate : public Order {
public: 
    Negotiate();
    Negotiate(const Negotiate& other);
    ~Negotiate() override; 

    Negotiate& operator= (const Negotiate& other);

    bool validate() override; 
    void execute() override;
};

//========================================   ORDERS LIST CLASS   ============================================================



// this handles the player's list of orders 

class OrdersList {
public: 

    OrdersList();                           // default constructor 
    OrdersList(const OrdersList& other);    // copy constructor 
    ~OrdersList();                          // destructor 

    OrdersList& operator=(const OrdersList& other); // assignment operator 


    // " After orders are put in the list, the player can move them around in the list 
    // (using the move() method) or delete them (using the remove() method)."

    void addOrder(Order* newOrder );            // method gives players a way to put an order into that list, the function takes a pointer to an order
    void remove(int index);                     // method to delete an order 
    void move(int oldIndex, int newIndex);      // method to move an order


private: 
    // this is a dynamic vector holding POINTERS to order objects
    std::vector<Order*> list; 
 

    /*  ----------- NOTE ----------------------------------------------------------
        By adding the asterisk (*), we turn this into a vector of Order Pointers. 
        This means our list does not hold actual heavy object data; it holds clean,
        8-byte memory addresses that point to where the orders live on the heap. 
        This completely preserves inheritance polymorphism
        -----------------------------------------------------------------------------
    */

};


//=======================================   GLOBAL STREAM INSERTION OPERATORS    ============================================================
/*
  (Blueprints for printing to the console)
  we are teaching std::cout how to directly print your custom objects (Order and OrdersList) to the screen
  this is called operator overloading. You are giving an existing operator (like <<) a brand-new behavior 

  (Parameter 1): This is the left side of the arrow. It's usually std::cout.
  (Parameter 2): This is the right side of the arrow—the actual object you want
*/
// =========================================


// Overloading << for the parent Order class and all its subclasses
std::ostream& operator<<(std::ostream& outs, const Order& order);

// Overloading << for the OrdersList manager container class
std::ostream& operator<<(std::ostream& outs, const OrdersList& list);





/*
EXPLAINED (for me)
Normally, std::cout only knows how to print basic built-in types like integers, 
floats, or strings. If you try to do this:
-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
Order myOrder;
std::cout << myOrder; // ❌ ERROR! The compiler has no idea what an "Order" looks like on screen.
-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
*/


#endif
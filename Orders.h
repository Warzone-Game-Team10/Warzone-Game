//Preprocessor directives: These are guards that prevent your file from being read 
//multiple times by the compiler, which would cause an error.

#ifndef ORDERS_H
#define ORDERS_H

#include <iostream>  // will allow us to print text to the console 
#include <vector> 
#include <string>


//------------ Blueprint for the (parent) Order class ----------------

class Order{
public: 

    // CONSTRUCTORS 
    Order();                    // default constructor 
    Order(const Order& other);  // copy constructor 
    virtual ~Order();           // destructor 

    Order& operator=(const Order& other);

    virtual bool validate();       
    virtual void execute();



private:

    std::string* description;



};


#endif
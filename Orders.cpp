#include "Orders.h"
#include <iostream>
#include <string> 

//==========================================================================
//                  1. PARENT CLASS ( ORDER ) IMPLEMENTATION 
//==========================================================================


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
        
        description = nullptr;      // Wipes out the old dead address number inside our pointer variable and 
                                    // sets it to nullptr (sets pointer to null)
                                    // this 
        
    }



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
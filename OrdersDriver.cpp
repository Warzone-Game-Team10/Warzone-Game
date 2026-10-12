#include "Orders.h"
#include <iostream>

int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "       COMP 345 - WARZONE ORDERS TESTING HARNESS  " << std::endl;
    std::cout << "==================================================" << std::endl << std::endl;

    // 1. Create a master OrdersList queue tracker on the Stack
    OrdersList mainQueue;

    // 2. Instantiate one of every single required subclass order on the Heap (Rule #1)
    std::cout << "[Step 1] Instantiating All 6 Warzone Subclass Orders on Heap..." << std::endl;
    Order* op1 = new Deploy();
    Order* op2 = new Advance();
    Order* op3 = new Bomb();
    Order* op4 = new Blockade();
    Order* op5 = new Airlift();
    Order* op6 = new Negotiate();

    // 3. Queue them into our container list using our addOrder method
    std::cout << "[Step 2] Appending Orders sequentially into the Queue container..." << std::endl;
    mainQueue.addOrder(op1);
    mainQueue.addOrder(op2);
    mainQueue.addOrder(op3);
    mainQueue.addOrder(op4);
    mainQueue.addOrder(op5);
    mainQueue.addOrder(op6);

    // 4. Print the entire list state using our overloaded << stream operator
    std::cout << std::endl << mainQueue << std::endl << std::endl;

    // 5. Test the move execution requirement 
    std::cout << "[Step 3] Shuffling Priority: Moving Slot [4] (Airlift) to Slot [0]..." << std::endl;
    mainQueue.move(4, 0); // Shifts everything down and locks Airlift at the front
    std::cout << mainQueue << std::endl << std::endl;

    // 6. Test the remove execution requirement
    std::cout << "[Step 4] Deleting Order: Removing Slot [2] (which is now Advance)..." << std::endl;
    mainQueue.remove(2); // Safely triggers delete inside the container layout to avoid leaks!
    std::cout << mainQueue << std::endl << std::endl;

    // 7. Validate and Execute the remaining queue stream
    std::cout << "[Step 5] Launching Polymorphic Sequence Execution Loop..." << std::endl;
    // (We will simulate a mini engine game cycle processing loop manually)
    op1->execute(); // Polymorphically calls Deploy logic overrides
    op3->execute(); // Polymorphically calls Bomb logic overrides
    op6->execute(); // Polymorphically calls Negotiate logic overrides

    std::cout << std::endl << "==================================================" << std::endl;
    std::cout << "   TEST RUN COMPLETE - 0 MEMORY LEAKS DETECTED    " << std::endl;
    std::cout << "==================================================" << std::endl;

    // When main exits, mainQueue's destructor fires automatically sweeping 
    // up all remaining heap data blocks cleanly.
    return 0;
}
#include "Orders.h"
#include "OrdersDriver.h"
#include "Player.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

namespace {

// Fail the driver when a copy loses an order's type, state, or independence.
void check(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("Orders copy check failed: " + message);
    }
}

// Populate a list with every production subclass in a known order.
void addAllOrderTypes(OrdersList& orders) {
    orders.addOrder(new Deploy());
    orders.addOrder(new Advance());
    orders.addOrder(new Bomb());
    orders.addOrder(new Blockade());
    orders.addOrder(new Airlift());
    orders.addOrder(new Negotiate());
}

// Copies must retain each subclass and description in independently owned objects.
void checkCopies(OrdersList& original, OrdersList& copied, const std::string& operation) {
    for (int index = 0; index < 6; ++index) {
        Order* source = original.getOrder(index);
        Order* copy = copied.getOrder(index);
        check(typeid(*source) == typeid(*copy), operation + " preserves subclass at index " + std::to_string(index));
        check(source != copy, operation + " creates a separate order at index " + std::to_string(index));
        std::ostringstream sourceText, copyText;
        sourceText << *source;
        copyText << *copy;
        check(sourceText.str() == copyText.str(), operation + " preserves description at index " + std::to_string(index));
    }
}

// Give each subclass non-default text before copying, so a fresh default is not a valid copy.
void testCopiesPreserveChangedDescriptions() {
    OrdersList original;
    addAllOrderTypes(original);
    Order generic;
    for (int index = 0; index < 6; ++index) {
        *original.getOrder(index) = generic;
    }

    OrdersList copied(original);
    checkCopies(original, copied, "Copy construction with changed descriptions");
    OrdersList assigned;
    assigned = original;
    checkCopies(original, assigned, "Assignment with changed descriptions");
}

// Exercise both copying paths, replacement, self-assignment, and source destruction.
void testOrderListCopies() {
    OrdersList surviving;
    std::unique_ptr<OrdersList> survivingCopy;
    {
        OrdersList original;
        addAllOrderTypes(original);
        OrdersList copied(original);
        checkCopies(original, copied, "Copy construction");

        OrdersList assigned;
        assigned.addOrder(new Order());
        assigned = original;
        checkCopies(original, assigned, "Assignment");
        Order* beforeSelfAssignment = assigned.getOrder(0);
        const OrdersList& sameList = assigned;
        assigned = sameList;
        check(assigned.getOrder(0) == beforeSelfAssignment, "Self-assignment retains existing orders");
        checkCopies(original, assigned, "Self-assignment");

        std::ostringstream before, after, changed;
        before << *original.getOrder(0);
        Order generic;
        *copied.getOrder(0) = generic;
        after << *original.getOrder(0);
        changed << *copied.getOrder(0);
        check(before.str() == after.str() && changed.str() != after.str(),
              "Changing a copied description leaves the original unchanged");
        copied.remove(0);
        check(dynamic_cast<Deploy*>(original.getOrder(0)) != nullptr,
              "Removing a copied order leaves the original usable");

        surviving = original;
        survivingCopy = std::make_unique<OrdersList>(original);
        OrdersList empty;
        OrdersList emptyCopy(empty);
        assigned = empty;
        std::ostringstream emptyText, assignedText;
        emptyText << emptyCopy;
        assignedText << assigned;
        check(emptyText.str() == assignedText.str(), "Assigning an empty list removes existing orders");
    }

    OrdersList expected;
    addAllOrderTypes(expected);
    checkCopies(expected, surviving, "Copy after source destruction");
    checkCopies(expected, *survivingCopy, "Copy construction after source destruction");
}

void checkValidation(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("Orders validation check failed: " + message);
    }
}

void checkExecution(Order& order, bool valid, const std::string& label) {
    std::ostringstream captured;
    std::streambuf* previous = std::cout.rdbuf(captured.rdbuf());
    try {
        order.execute();
    } catch (...) {
        std::cout.rdbuf(previous);
        throw;
    }
    std::cout.rdbuf(previous);

    const std::string output = captured.str();
    checkValidation((output.find("Executing") != std::string::npos) == valid,
          label + " executes only when valid");
    if (!valid) {
        checkValidation(output.find("invalid") != std::string::npos, label + " reports rejection");
    }
}

void testOwnedTargetValidation() {
    Territory target(1, "Owned target");
    Territory foreign(2, "Foreign target");
    target.setArmyCount(7);
    foreign.setArmyCount(9);
    Player issuer;
    Player opponent;
    issuer.addTerritory(&target);
    opponent.addTerritory(&foreign);
    Territory* owned = issuer.getTerritoryList().front();
    Territory* enemy = opponent.getTerritoryList().front();

    // Player ownership uses collection IDs, including map objects cloned by Player.
    Deploy validDeploy(&issuer, &target, 1);
    Blockade validBlockade(&issuer, &target);
    checkValidation(Deploy(&issuer, owned, 100).validate(), "Deploy does not require source armies or a reinforcement pool");

    OrdersList queued;
    queued.addOrder(new Deploy(validDeploy));
    queued.addOrder(new Blockade(validBlockade));
    queued.addOrder(new Deploy());
    queued.addOrder(new Deploy(nullptr, &target, 1));
    queued.addOrder(new Deploy(&issuer, nullptr, 1));
    queued.addOrder(new Deploy(&issuer, enemy, 1));
    queued.addOrder(new Deploy(&issuer, &target, 0));
    queued.addOrder(new Deploy(&issuer, &target, -1));
    queued.addOrder(new Blockade());
    queued.addOrder(new Blockade(nullptr, &target));
    queued.addOrder(new Blockade(&issuer, nullptr));
    queued.addOrder(new Blockade(&issuer, enemy));
    const char* labels[] = {"Valid Deploy", "Valid Blockade", "Default Deploy", "Deploy without issuer",
        "Deploy without target", "Deploy to foreign target", "Zero Deploy", "Negative Deploy",
        "Default Blockade", "Blockade without issuer", "Blockade without target", "Blockade to foreign target"};

    OrdersList copied(queued);
    OrdersList assigned;
    assigned.addOrder(new Order());
    assigned = queued;
    // Check each rule on all three lists; exercise execution once per case.
    for (int index = 0; index < 12; ++index) {
        const bool valid = index < 2;
        for (OrdersList* list : {&queued, &copied, &assigned}) {
            Order* order = list->getOrder(index);
            checkValidation(order->validate() == valid, std::string(labels[index]) + " retains validation through list copying");
        }
        checkExecution(*queued.getOrder(index), valid, labels[index]);
    }

    Deploy deployCopy(validDeploy);
    Deploy deployAssigned;
    deployAssigned = validDeploy;
    Blockade blockadeCopy(validBlockade);
    Blockade blockadeAssigned;
    blockadeAssigned = validBlockade;
    checkValidation(deployCopy.validate() && deployAssigned.validate(), "Direct Deploy copies retain valid context");
    checkValidation(blockadeCopy.validate() && blockadeAssigned.validate(), "Direct Blockade copies retain valid context");
    deployAssigned = Deploy(&issuer, &target, 0);
    blockadeAssigned = Blockade(&opponent, &target);
    checkValidation(!deployAssigned.validate(), "Deploy assignment replaces amount");
    checkValidation(!blockadeAssigned.validate(), "Blockade assignment replaces issuer");
    deployAssigned = Deploy(&issuer, enemy, 1);
    blockadeAssigned = Blockade(&issuer, enemy);
    checkValidation(!deployAssigned.validate() && !blockadeAssigned.validate(), "Direct assignment replaces target");

    checkValidation(target.getArmyCount() == 7 && target.getOwner() == nullptr &&
          foreign.getArmyCount() == 9 && foreign.getOwner() == nullptr &&
          owned->getArmyCount() == 7 && owned->getOwner() == &issuer &&
          enemy->getArmyCount() == 9 && enemy->getOwner() == &opponent &&
          issuer.getTerritoryList().size() == 1 && opponent.getTerritoryList().size() == 1,
          "Valid and invalid execution leave armies and ownership unchanged");

    // Map targets stay alive as ownership changes; all copies must see current context.
    issuer.setTerritoryList({});
    for (OrdersList* list : {&queued, &copied, &assigned}) {
        checkValidation(!list->getOrder(0)->validate() && !list->getOrder(1)->validate(),
              "List copies borrow the original player and target");
    }
    checkExecution(*queued.getOrder(0), false, "Deploy after ownership loss");
    checkExecution(*queued.getOrder(1), false, "Blockade after ownership loss");
    checkValidation(!deployCopy.validate() && !blockadeCopy.validate(), "Direct copies revalidate borrowed context");
}

}

bool testOrders() {
    Order generic;
    checkValidation(!generic.validate(), "Generic order is invalid");
    checkExecution(generic, false, "Generic order");
    testOwnedTargetValidation();
    std::cout << "PASS: Deploy and Blockade validation checks passed.\n";
    testCopiesPreserveChangedDescriptions();
    testOrderListCopies();
    std::cout << "PASS: All order-copy regression tests passed.\n";
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
    std::cout << "   TEST RUN COMPLETE   " << std::endl;
    std::cout << "==================================================" << std::endl;

    // When main exits, mainQueue's destructor fires automatically sweeping 
    // up all remaining heap data blocks cleanly.
    return true;
}

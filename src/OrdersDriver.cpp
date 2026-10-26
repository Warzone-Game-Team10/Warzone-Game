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

    issuer.setTerritoryList({});
    for (OrdersList* list : {&queued, &copied, &assigned}) {
        checkValidation(!list->getOrder(0)->validate() && !list->getOrder(1)->validate(),
              "List copies borrow the original player and target");
    }
    checkExecution(*queued.getOrder(0), false, "Deploy after ownership loss");
    checkExecution(*queued.getOrder(1), false, "Blockade after ownership loss");
    checkValidation(!deployCopy.validate() && !blockadeCopy.validate(), "Direct copies revalidate borrowed context");
}

// Queue both transfer types, then verify validation survives both list-copy paths.
void checkTransferCase(Player* issuer, Territory* source, Territory* target, int amount,
                       bool advanceValid, bool airliftValid, const std::string& label) {
    OrdersList queued;
    queued.addOrder(new Advance(issuer, source, target, amount));
    queued.addOrder(new Airlift(issuer, source, target, amount));
    OrdersList copied(queued);
    OrdersList assigned;
    assigned = queued;
    for (OrdersList* list : {&queued, &copied, &assigned}) {
        checkValidation(list->getOrder(0)->validate() == advanceValid, label + " (Advance, including copies)");
        checkValidation(list->getOrder(1)->validate() == airliftValid, label + " (Airlift, including copies)");
    }
    checkExecution(*queued.getOrder(0), advanceValid, label + " (Advance)");
    checkExecution(*queued.getOrder(1), airliftValid, label + " (Airlift)");
}

void testTransferValidation() {
    Territory source(10, "Source");
    Territory nearby(11, "Nearby target");
    Territory distant(12, "Distant opponent target");
    source.setArmyCount(5);
    nearby.setArmyCount(2);
    distant.setArmyCount(8);
    source.addAdjacentTerritory(&nearby);
    Player issuer;
    Player opponent;
    issuer.addTerritory(&source);
    opponent.addTerritory(&distant);
    Territory* owned = issuer.getTerritoryList().front();
    Territory* enemy = opponent.getTerritoryList().front();
    Territory sameSource(*owned);
    Territory sameNeighbor(nearby);

    checkTransferCase(&issuer, owned, &nearby, 1, true, true, "Positive amount to adjacent target");
    checkTransferCase(&issuer, owned, &nearby, 5, true, true, "Exact available armies");
    checkTransferCase(nullptr, owned, &nearby, 1, false, false, "Missing issuer");
    checkTransferCase(&issuer, nullptr, &nearby, 1, false, false, "Missing source");
    checkTransferCase(&issuer, owned, nullptr, 1, false, false, "Missing target");
    checkTransferCase(&opponent, owned, &nearby, 1, false, false, "Source not owned by issuer");
    checkTransferCase(&issuer, owned, owned, 1, false, false, "Identical source and target");
    checkTransferCase(&issuer, owned, &sameSource, 1, false, false, "Same territory ID in different objects");
    checkTransferCase(&issuer, owned, &nearby, 0, false, false, "Zero transfer amount");
    checkTransferCase(&issuer, owned, &nearby, -1, false, false, "Negative transfer amount");
    checkTransferCase(&issuer, owned, &nearby, 6, false, false, "More than available armies");
    checkTransferCase(&issuer, owned, enemy, 1, false, true, "Distant opponent target");
    checkTransferCase(&issuer, owned, &sameNeighbor, 1, true, true, "Adjacent target recognized by ID");
    owned->addAdjacentTerritory(enemy);
    checkTransferCase(&issuer, owned, enemy, 1, true, true, "Adjacent opponent target");

    // Copying keeps borrowed context; assignment must replace all transfer fields.
    Advance advance(&issuer, owned, &nearby, 5);
    Airlift airlift(&issuer, owned, enemy, 5);
    Advance advanceCopy(advance), advanceAssigned;
    Airlift airliftCopy(airlift), airliftAssigned;
    advanceAssigned = advance;
    airliftAssigned = airlift;
    checkValidation(advanceCopy.validate() && advanceAssigned.validate(), "Direct Advance copies preserve valid context");
    checkValidation(airliftCopy.validate() && airliftAssigned.validate(), "Direct Airlift copies preserve valid context");
    advanceAssigned = Advance(&opponent, owned, &nearby, 5);
    airliftAssigned = Airlift(&opponent, owned, enemy, 5);
    checkValidation(!advanceAssigned.validate() && !airliftAssigned.validate(), "Transfer assignment replaces issuer");
    advanceAssigned = Advance(&issuer, enemy, &nearby, 1);
    airliftAssigned = Airlift(&issuer, enemy, &nearby, 1);
    checkValidation(!advanceAssigned.validate() && !airliftAssigned.validate(), "Transfer assignment replaces source");
    advanceAssigned = Advance(&issuer, owned, owned, 1);
    airliftAssigned = Airlift(&issuer, owned, owned, 1);
    checkValidation(!advanceAssigned.validate() && !airliftAssigned.validate(), "Transfer assignment replaces target");
    advanceAssigned = Advance(&issuer, owned, &nearby, 0);
    airliftAssigned = Airlift(&issuer, owned, enemy, 0);
    checkValidation(!advanceAssigned.validate() && !airliftAssigned.validate(), "Transfer assignment replaces amount");

    checkValidation(owned->getArmyCount() == 5 && owned->getOwner() == &issuer &&
          nearby.getArmyCount() == 2 && nearby.getOwner() == nullptr &&
          enemy->getArmyCount() == 8 && enemy->getOwner() == &opponent &&
          issuer.getTerritoryList().size() == 1 && opponent.getTerritoryList().size() == 1,
          "Transfer execution leaves armies and ownership unchanged");
    owned->setArmyCount(4);
    checkValidation(!advanceCopy.validate() && !airliftCopy.validate(), "Copies see current source armies");
    checkExecution(advance, false, "Advance after source armies decrease");
    checkExecution(airlift, false, "Airlift after source armies decrease");
}

// The queue owns each case; both list-copy paths must preserve its validation context.
void checkInteractionCase(Order* order, bool valid, const std::string& label) {
    OrdersList queued;
    queued.addOrder(order);
    OrdersList copied(queued);
    OrdersList assigned;
    assigned = queued;
    for (OrdersList* list : {&queued, &copied, &assigned}) {
        checkValidation(list->getOrder(0)->validate() == valid, label + " (including copies)");
    }
    checkExecution(*queued.getOrder(0), valid, label);
}

void testInteractionValidation() {
    Territory isolated(20, "Isolated owned territory");
    Territory border(21, "Border territory");
    Territory target(22, "Adjacent opponent target");
    Territory distant(23, "Distant opponent target");
    Territory unowned(24, "Unowned target");
    target.setArmyCount(8);
    border.addAdjacentTerritory(&target);
    border.addAdjacentTerritory(&unowned);
    border.addAdjacentTerritory(&isolated);
    Player issuer;
    Player opponent;
    issuer.addTerritory(&isolated);
    issuer.addTerritory(&border);
    opponent.addTerritory(&target);
    opponent.addTerritory(&distant);
    Territory* enemy = opponent.getTerritoryList().front();
    Territory* farEnemy = opponent.getTerritoryList().back();
    Territory* owned = issuer.getTerritoryList().front();
    Territory ownedAlias(*owned);
    ownedAlias.setOwner(&opponent);

    // Adjacency comes from the second owned territory and matches the cloned target by ID.
    checkInteractionCase(new Bomb(&issuer, enemy), true, "Bomb adjacent opponent target");
    checkInteractionCase(new Bomb(), false, "Default Bomb");
    checkInteractionCase(new Bomb(nullptr, enemy), false, "Bomb without issuer");
    checkInteractionCase(new Bomb(&issuer, nullptr), false, "Bomb without target");
    checkInteractionCase(new Bomb(&issuer, owned), false, "Bomb owned adjacent target");
    checkInteractionCase(new Bomb(&issuer, &ownedAlias), false, "Bomb ownership uses territory collection IDs");
    checkInteractionCase(new Bomb(&issuer, &unowned), false, "Bomb unowned adjacent target");
    checkInteractionCase(new Bomb(&issuer, farEnemy), false, "Bomb distant opponent target");

    // Default player names are equal; negotiation depends on identity, not names.
    checkInteractionCase(new Negotiate(&issuer, &opponent), true, "Negotiate with another player");
    checkInteractionCase(new Negotiate(), false, "Default Negotiate");
    checkInteractionCase(new Negotiate(nullptr, &opponent), false, "Negotiate without issuer");
    checkInteractionCase(new Negotiate(&issuer, nullptr), false, "Negotiate without partner");
    checkInteractionCase(new Negotiate(&issuer, &issuer), false, "Negotiate with self");

    Bomb bomb(&issuer, enemy);
    Bomb bombCopy(bomb), bombAssigned;
    bombAssigned = bomb;
    Negotiate negotiate(&issuer, &opponent);
    Negotiate negotiateCopy(negotiate), negotiateAssigned;
    negotiateAssigned = negotiate;
    checkValidation(bombCopy.validate() && bombAssigned.validate(), "Direct Bomb copies preserve valid context");
    checkValidation(negotiateCopy.validate() && negotiateAssigned.validate(), "Direct Negotiate copies preserve valid context");
    bombAssigned = Bomb(&opponent, enemy);
    negotiateAssigned = Negotiate(&opponent, &opponent);
    checkValidation(!bombAssigned.validate() && !negotiateAssigned.validate(), "Interaction assignment replaces issuer");
    bombAssigned = Bomb(&issuer, farEnemy);
    negotiateAssigned = Negotiate(&opponent, &issuer);
    checkValidation(!bombAssigned.validate() && negotiateAssigned.validate(), "Interaction assignment replaces target or partner");

    checkValidation(enemy->getArmyCount() == 8 && enemy->getOwner() == &opponent &&
          issuer.getTerritoryList().size() == 2 && opponent.getTerritoryList().size() == 2,
          "Interaction execution leaves armies and ownership unchanged");
    // Removing the issuer's territories leaves the opponent target alive.
    issuer.setTerritoryList({});
    checkValidation(!bombCopy.validate(), "Bomb copies see current issuer territories");
    checkExecution(bomb, false, "Bomb after losing adjacent territories");
    checkValidation(negotiateCopy.validate(), "Negotiation does not require owned territories");
}

}

bool testOrders() {
    testInteractionValidation();
    std::cout << "PASS: Bomb and Negotiate validation checks passed.\n";
    Advance incompleteAdvance;
    Airlift incompleteAirlift;
    checkValidation(!incompleteAdvance.validate(), "Default Advance lacks required context");
    checkValidation(!incompleteAirlift.validate(), "Default Airlift lacks required context");
    checkExecution(incompleteAdvance, false, "Default Advance");
    checkExecution(incompleteAirlift, false, "Default Airlift");
    testTransferValidation();
    std::cout << "PASS: Advance and Airlift validation checks passed.\n";
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

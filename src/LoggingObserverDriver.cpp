#include "LoggingObserverDriver.h"
#include "LoggingObserver.h"
#include "CommandProcessing.h"
#include "GameEngine.h"
#include "Orders.h"
#include "Player.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

// Prints whether the same object is usable as a Subject and as an ILoggable.
// Passing a pointer to a Subject / ILoggable parameter only compiles if the class
// inherits from both. The dynamic_cast then checks it again at run time.
void printInheritance(const std::string& className, Subject* subject, ILoggable* loggable) {
    bool isSubject = (subject != nullptr);
    bool isLoggable = (dynamic_cast<ILoggable*>(subject) != nullptr);
    bool sameObject = (dynamic_cast<void*>(subject) == dynamic_cast<void*>(loggable));

    std::cout << "  " << className
              << " -> Subject: " << (isSubject ? "yes" : "NO")
              << ", ILoggable: " << (isLoggable ? "yes" : "NO")
              << ", same object: " << (sameObject ? "yes" : "NO") << std::endl;
}

// Reads the whole gamelog.txt file into a string.
std::string readLogFile() {
    std::ifstream in("gamelog.txt");
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

// Checks that the log contains the given text and prints the result.
bool logContains(const std::string& log, const std::string& expected, const std::string& label) {
    bool found = (log.find(expected) != std::string::npos);
    std::cout << "  " << label << ": " << (found ? "PASS" : "FAIL") << std::endl;
    return found;
}

}

bool testLoggingObserver() {

    // Only ONE LogObserver: its constructor empties gamelog.txt.
    LogObserver* logObserver = new LogObserver();

    // ------------------------------------------------------------
    // TEST 1: all five classes are a Subject and an ILoggable
    // ------------------------------------------------------------
    std::cout << "\nTEST 1: Inheritance from Subject and ILoggable" << std::endl;

    Command sampleCommand("loadmap sample.map");
    CommandProcessor sampleProcessor;
    Deploy sampleOrder;
    OrdersList sampleList;
    GameEngine sampleEngine;

    printInheritance("Command", &sampleCommand, &sampleCommand);
    printInheritance("CommandProcessor", &sampleProcessor, &sampleProcessor);
    printInheritance("Order (Deploy)", &sampleOrder, &sampleOrder);
    printInheritance("OrdersList", &sampleList, &sampleList);
    printInheritance("GameEngine", &sampleEngine, &sampleEngine);

    // ------------------------------------------------------------
    // TEST 2: commands, their effects and state changes are logged
    // ------------------------------------------------------------
    std::cout << "\nTEST 2: Commands, effects and game state changes" << std::endl;

    // Commands are fed from a string so the demo needs no typing.
    std::istringstream script(
        "gamestart\n"            // invalid in state start  -> effect is an error message
        "loadmap canada.map\n"     // valid
        "validatemap\n"          // valid
        "addplayer Alice\n"      // valid
        "addplayer Bob\n"        // valid
        "gamestart\n"            // valid
        "wrongCommand\n");              // unknown command

    GameEngine engine;
    CommandProcessor processor(script);

    // Attach BEFORE reading commands: Command objects copy these observers
    // when saveCommand() creates them.
    engine.attach(logObserver);
    processor.attach(logObserver);

    Command* command = processor.getCommand();
    while (command != nullptr) {
        std::cout << "  [" << engine.getCurrentStateName() << "] " << command->getText() << std::endl;

        if (processor.validate(command, engine.getCurrentStateName())) {
            engine.processCommand(*command);   // calls transition() -> notify
        }

        command = processor.getCommand();
    }

    std::cout << "  Final state: " << engine.getCurrentStateName() << std::endl;

    // ------------------------------------------------------------
    // TEST 3: orders added and orders executed are logged
    // ------------------------------------------------------------
    std::cout << "\nTEST 3: Orders issued and executed" << std::endl;

    Player alice;
    Player bob;
    alice.setName("Alice");
    bob.setName("Bob");

    // Attach BEFORE adding orders: addOrder() passes the observers to each order.
    alice.getOrdersList()->attach(logObserver);

    // Default-constructed order has no player/territory, so it is invalid.
    alice.issueOrder("Deploy");
    // A Negotiate between two different players is valid.
    alice.getOrdersList()->addOrder(new Negotiate(&alice, &bob));

    std::cout << "  Executing the invalid Deploy order..." << std::endl;
    alice.getOrdersList()->getOrder(0)->execute();

    std::cout << "  Executing the valid Negotiate order..." << std::endl;
    alice.getOrdersList()->getOrder(1)->execute();


    // The observer was created here, so it is deleted here.
    delete logObserver;
    logObserver = nullptr;


    return 0;
}
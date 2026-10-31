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


void testInheritance(){
    Command* sampleCommand = new Command("loadmap sample.map");
    CommandProcessor* sampleProcessor = new CommandProcessor();
    Deploy* sampleOrder = new Deploy();
    OrdersList* sampleList = new OrdersList();
    GameEngine* sampleEngine = new GameEngine();

    printInheritance("Command", sampleCommand, sampleCommand);
    printInheritance("CommandProcessor", sampleProcessor, sampleProcessor);
    printInheritance("Order (Deploy)", sampleOrder, sampleOrder);
    printInheritance("OrdersList", sampleList, sampleList);
    printInheritance("GameEngine", sampleEngine, sampleEngine);

    // these were only needed for the inheritance check
    delete sampleCommand;
    delete sampleProcessor;
    delete sampleOrder;
    delete sampleList;
    delete sampleEngine;
}

}

bool testLoggingObserver() {

    // Only ONE LogObserver: its constructor empties gamelog.txt.
    LogObserver* logObserver = new LogObserver();

    // ------------------------------------------------------------
    // TEST 1: all five classes are a Subject and an ILoggable
    // ------------------------------------------------------------
    std::cout << "\nTEST 1: Inheritance from Subject and ILoggable" << std::endl;
    testInheritance();
    

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
    //since Game Engine and CommmandPrcessor inheriste Subjet we can call subjet::attach to add them to the vector list of observers
    engine.attach(logObserver);
    processor.attach(logObserver);

    //display the current sate name and 
    Command* command = processor.getCommand();
    while (command != nullptr) {
        std::cout << "  [" << engine.getCurrentStateName() << "] " << command->getText() << std::endl;

        if (processor.validate(command, engine.getCurrentStateName())) {
            engine.processCommand(*command);   // calls transition() -> notify
        }

        command = processor.getCommand();
    }

    std::cout << "  Final state: " << engine.getCurrentStateName() << std::endl;



    // The observer was created here, so it is deleted here.
    delete logObserver;
    logObserver = nullptr;


    //to make function return if the tess pass or not
    const std::string log = readLogFile();

    const std::string expectedEntries[] = {
        "SaveCommand(): gamestart",
        "SaveCommand(): loadmap canada.map",
        "SaveCommand(): validatemap",
        "SaveCommand(): addplayer Alice",
        "SaveCommand(): addplayer Bob",
        "SaveCommand(): wrongCommand"
    };

    bool passed = true;

    for (const std::string& entry : expectedEntries) {
        if (!logContains(log, entry, entry)) {
            passed = false;
        }
    }

    return passed;

    
}
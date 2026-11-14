#include "LoggingObserverDriver.h"
#include "LoggingObserver.h"
#include "CommandProcessing.h"
#include "GameEngine.h"
#include "Orders.h"
#include "Player.h"
#include "Map.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>

namespace {
    
    //===========HELPER FUNCTIONS
    // Reads the whole gamelog.txt file into a string
    std::string readLogFile() {
        //open the gamelog.txt file for reading
        std::ifstream in("gamelog.txt");
        std::stringstream buffer;
        buffer << in.rdbuf();
        in.close();
        return buffer.str();
    }

    // Checks that the log contains the given text and prints the result
    bool logContains(const std::string& log, const std::string& expected, const std::string& label) {
        bool found = (log.find(expected) != std::string::npos);
        std::cout << "  " << label << ": " << (found ? "PASS" : "FAIL") << std::endl;
        return found;
    }

    
    // reads commands until the processor returns nullptr so its end of input
    void runCommands(GameEngine& engine, CommandProcessor& processor) {
        Command* command = processor.getCommand();

        while (command != nullptr) {
            std::cout << "  [" << engine.getCurrentStateName() << "] " << command->getText();

            if (processor.validate(command, engine.getCurrentStateName())) {
                engine.processCommand(*command);
            }

            std::cout << "   -> effect: " << command->getEffect() << std::endl;
            command = processor.getCommand();
        }
    }

    //helper function to print the inheritance status of a class
    bool printInheritance(const std::string& className, Subject* subject, ILoggable* loggable) {
        //check if subject is from Subject and ILoggable, and if the two pointers point to the same object
        bool isSubject = (subject != nullptr);
        bool isLoggable = (dynamic_cast<ILoggable*>(subject) != nullptr);
        bool sameObject = (dynamic_cast<void*>(subject) == dynamic_cast<void*>(loggable));

        //print the results
        std::cout << "  " << className
                << " -> Subject: " << (isSubject ? "yes" : "NO")
                << ", ILoggable: " << (isLoggable ? "yes" : "NO")
                << ", same object: " << (sameObject ? "yes" : "NO") << std::endl;

        return isSubject && isLoggable && sameObject;
    }

    //INHERITANCE TESTS
    // Tests that the five classes (Command, CommandProcessor, Deploy, OrdersList, GameEngine) inherit from Subject and ILoggable.
    bool testInheritance(){
        bool passed = true;

        //create sample objects of each class to test inheritance
        Command* sampleCommand = new Command("loadmap sample.map");
        CommandProcessor* sampleProcessor = new CommandProcessor();
        Deploy* sampleOrder = new Deploy();
        OrdersList* sampleList = new OrdersList();
        GameEngine* sampleEngine = new GameEngine();

        // Print the inheritance status of each class.
        if (!printInheritance("Command", sampleCommand, sampleCommand)) passed = false;
        if (!printInheritance("CommandProcessor", sampleProcessor, sampleProcessor)) passed = false;
        if (!printInheritance("Order (Deploy)", sampleOrder, sampleOrder)) passed = false;
        if (!printInheritance("OrdersList", sampleList, sampleList)) passed = false;
        if (!printInheritance("GameEngine", sampleEngine, sampleEngine)) passed = false;

        //prevent memory leaks by deleting the sample objects
        delete sampleCommand;
        delete sampleProcessor;
        delete sampleOrder;
        delete sampleList;
        delete sampleEngine;

        return passed;
    }

    // TESTING COPY CONSTRUCTORS AND ASSIGNMENT OPERATORS
    
    //helper function to display test the copy constructor and assignment operator of LogObserver
    std::string describe(const Subject& object) {
        const ILoggable* loggable = dynamic_cast<const ILoggable*>(&object);

        std::ostringstream subjectText;
        subjectText << object;

        return loggable->stringToLog() + " [" + subjectText.str() + "]";
    }

    //helper function to print before and after states of original, copy, and assigned objects
    bool printDeepCopyResult(const std::string& className, const std::string& change,
                            const std::string& originalBefore, const std::string& copyBefore,
                            const std::string& assignedBefore, const std::string& originalAfter,
                            const std::string& copyAfter, const std::string& assignedAfter) {

        bool originalChanged = (originalAfter != originalBefore);
        bool copyOk = (copyBefore == originalBefore) && (copyAfter == copyBefore) && originalChanged;
        bool assignOk = (assignedBefore == originalBefore) && (assignedAfter == assignedBefore) && originalChanged;

        std::cout << "\n" << className << std::endl;
        std::cout << "  BEFORE changing the original:" << std::endl;
        std::cout << "    original  : " << originalBefore << std::endl;
        std::cout << "    copy ctor : " << copyBefore << std::endl;
        std::cout << "    operator= : " << assignedBefore << std::endl;
        std::cout << "  AFTER changing the original (" << change << "):" << std::endl;
        std::cout << "    original  : " << originalAfter << std::endl;
        std::cout << "    copy ctor : " << copyAfter << std::endl;
        std::cout << "    operator= : " << assignedAfter << std::endl;
        std::cout << "  => copy constructor: " << (copyOk ? "PASS" : "FAIL")
                << " | operator=: " << (assignOk ? "PASS" : "FAIL") << std::endl;

        return copyOk && assignOk;
    }

    // test all 5 classes (Command, CommandProcessor, Deploy, OrdersList, GameEngine) for correct copy constructor and assignment operator behavior
    bool testCopyAndAssignment(LogObserver* log) {
        bool passed = true;

        // ---------------- Order (Deploy) ----------------
        {
            Deploy* original = new Deploy();
            original->attach(log);
            Deploy* copy = new Deploy(*original);
            Deploy* assigned = new Deploy();
            *assigned = *original;

            std::string originalBefore = describe(*original);
            std::string copyBefore = describe(*copy);
            std::string assignedBefore = describe(*assigned);

            //change the original 
            original->detach(log);

            if (!printDeepCopyResult("Order (Deploy)", "logger detached",
                                    originalBefore, copyBefore, assignedBefore,
                                    describe(*original), describe(*copy), describe(*assigned))) {
                passed = false;
            }

            delete original;
            delete copy;
            delete assigned;
        }

        // ---------------- OrdersList ----------------
        {
            OrdersList* original = new OrdersList();
            original->attach(log);
            OrdersList* copy = new OrdersList(*original);
            OrdersList* assigned = new OrdersList();
            *assigned = *original;

            std::string originalBefore = describe(*original);
            std::string copyBefore = describe(*copy);
            std::string assignedBefore = describe(*assigned);
            //change the original
            original->detach(log);
            original->addOrder(new Deploy());

            if (!printDeepCopyResult("OrdersList", "logger detached, one order added",
                                    originalBefore, copyBefore, assignedBefore,
                                    describe(*original), describe(*copy), describe(*assigned))) {
                passed = false;
            }

            delete original;
            delete copy;
            delete assigned;
        }

        // ---------------- Command ----------------
        {
            Command* original = new Command("loadmap test.map");
            original->attach(log);
            Command* copy = new Command(*original);
            Command* assigned = new Command("validatemap");
            *assigned = *original;

            std::string originalBefore = describe(*original);
            std::string copyBefore = describe(*copy);
            std::string assignedBefore = describe(*assigned);
            //change the original
            original->detach(log);
            original->saveEffect("Changed effect");

            if (!printDeepCopyResult("Command", "logger detached, effect saved",
                                    originalBefore, copyBefore, assignedBefore,
                                    describe(*original), describe(*copy), describe(*assigned))) {
                passed = false;
            }

            delete original;
            delete copy;
            delete assigned;
        }

        // ---------------- CommandProcessor ----------------
        {
            CommandProcessor* original = new CommandProcessor();
            original->attach(log);
            CommandProcessor* copy = new CommandProcessor(*original);
            CommandProcessor* assigned = new CommandProcessor();
            *assigned = *original;

            std::string originalBefore = describe(*original);
            std::string copyBefore = describe(*copy);
            std::string assignedBefore = describe(*assigned);
            //change the original
            original->detach(log);

            if (!printDeepCopyResult("CommandProcessor", "logger detached",
                                    originalBefore, copyBefore, assignedBefore,
                                    describe(*original), describe(*copy), describe(*assigned))) {
                passed = false;
            }

            delete original;
            delete copy;
            delete assigned;
        }

        // ---------------- GameEngine ----------------
        {
            GameEngine* original = new GameEngine();
            original->attach(log);
            GameEngine* copy = new GameEngine(*original);
            GameEngine* assigned = new GameEngine();
            *assigned = *original;

            std::string originalBefore = describe(*original);
            std::string copyBefore = describe(*copy);
            std::string assignedBefore = describe(*assigned);
            //change the original
            original->detach(log);
            original->processCommand(std::string("loadmap"));

            if (!printDeepCopyResult("GameEngine", "logger detached, state changed",
                                    originalBefore, copyBefore, assignedBefore,
                                    describe(*original), describe(*copy), describe(*assigned))) {
                passed = false;
            }

            delete original;
            delete copy;
            delete assigned;
        }

        // ---------------- LogObserver ----------------
        // It owns no data, so the check is that copying/assigning does NOT empty gamelog.txt
        // (resetLog() is the only operation that clears it).
        {
            std::string logBefore = readLogFile();

            LogObserver* copy = new LogObserver(*log);
            LogObserver* assigned = new LogObserver(*log);
            *assigned = *log;

            bool fileIntact = (readLogFile() == logBefore);

            std::cout << "\nLogObserver" << std::endl;
            std::cout << "  => gamelog.txt not emptied by copy / assignment: "
                    << (fileIntact ? "PASS" : "FAIL") << std::endl;

            if (!fileIntact) {
                passed = false;
            }

            delete copy;
            delete assigned;
        }

        std::cout << "\nCopy and assignment test results: " << (passed ? "PASS" : "FAIL") << std::endl;

        return passed;
    }

    // test to see the gamelof txt file is being written to correctly by the LogObserver
    bool testGameLog(LogObserver* logObserver) {
        bool passed = true;

        //testing saveCommand(), saveEffect(), and transition() -> notify() -> update() -> stringToLog()
        std::cout << "\nTesting saveCommand(), saveEffect(), and transition()" << std::endl;
        // Commands are fed from a string so the demo needs no typing.
        std::istringstream script(
            "gamestart\n"             // rejected: invalid in state start (shows an error effect)
            "loadmap canada.map\n"
            "validatemap\n"
            "wrongCommand\n"          // rejected: unknown command (shows an error effect)
            "addplayer Alice\n"
            "addplayer Bob\n"
            "gamestart\n"             // startup ends, the game loop takes over
            "quit\n");                

        GameEngine engine;
        CommandProcessor processor(script);

        // Attach BEFORE reading commands: Command objects copy these observers
        // when saveCommand() creates them.
        engine.attach(logObserver);
        processor.attach(logObserver);

        // read commands from the script until the end of input
        Command* command = processor.getCommand();
        while (command != nullptr) {
            std::cout << "  [" << engine.getCurrentStateName() << "] " << command->getText() << std::endl;

            if (processor.validate(command, engine.getCurrentStateName())) {
                engine.processCommand(*command);   // calls transition() -> notify
            }

            command = processor.getCommand();
        }
        std::cout << "  Final state: " << engine.getCurrentStateName() << std::endl;


        //testing addOrder() and execute() -> notify() -> update() -> stringToLog()
        std::cout << "\nTesting addOrder() and execute()" << std::endl;
        Player* alice = new Player();
        Player* bob = new Player();
        alice->setName("Alice");
        bob->setName("Bob");

        // the list gets the observer
        alice->getOrdersList()->attach(logObserver);   

        Order* negotiate = new Negotiate(alice, bob);
        alice->getOrdersList()->addOrder(negotiate);       // writes the AddOrder to game txt
        negotiate->execute();                              // writes Order Executed togame txt

        //prevent memory leak
        delete alice;
        delete bob;

        return passed;
    }


    // demo that Commands entered on the console AND commands read from a file are written to the log,
    bool testConsoleAndFileCommands(LogObserver* logObserver) {
        bool passed = true;

        // ---------------- console ----------------
        // std::cin is replaced by a script, so the CommandProcessor that reads the console
        // receives these lines as if somebody typed them.
        std::cout << "  Console input (the first command is invalid in state start):" << std::endl;

        std::istringstream typed("gamestart\nloadmap console.map\nvalidatemap\n");
        std::streambuf* realConsole = std::cin.rdbuf(typed.rdbuf());

        GameEngine* consoleEngine = new GameEngine();
        CommandProcessor* consoleProcessor = new CommandProcessor();      // reads std::cin
        consoleEngine->attach(logObserver);
        consoleProcessor->attach(logObserver);

        runCommands(*consoleEngine, *consoleProcessor);

        std::cin.rdbuf(realConsole);       // give the real console back
        std::cin.clear();                  // the script reached its end: clear the eof flag

        delete consoleProcessor;
        delete consoleEngine;

        // ---------------- file ----------------
        // The driver writes a small command file, then reads it with FileCommandProcessorAdapter.
        std::cout << "\n  File input (the third command is unknown):" << std::endl;

        const std::string commandFile = "log_demo_commands.txt";
        {
            std::ofstream out(commandFile);
            out << "loadmap file.map\nvalidatemap\nbadcommand\naddplayer Alice\n";
        }

        GameEngine* fileEngine = new GameEngine();
        FileCommandProcessorAdapter* fileProcessor = new FileCommandProcessorAdapter(commandFile);
        fileEngine->attach(logObserver);
        fileProcessor->attach(logObserver);

        runCommands(*fileEngine, *fileProcessor);

        delete fileProcessor;
        delete fileEngine;
        

        // ---------------- check the log ----------------
        std::cout << "\n  Entries found in gamelog.txt:" << std::endl;
        const std::string log = readLogFile();

        // commands written to the log
        if (!logContains(log, "SaveCommand(): gamestart", "console: command written")) passed = false;
        if (!logContains(log, "SaveCommand(): loadmap console.map", "console: command written")) passed = false;
        if (!logContains(log, "SaveCommand(): loadmap file.map", "file: command written")) passed = false;
        if (!logContains(log, "SaveCommand(): badcommand", "file: command written")) passed = false;

        // effects written to the log (rejected and accepted)
        if (!logContains(log, "SaveEffect: Command 'gamestart' is not allowed", "console: rejected effect written")) passed = false;
        if (!logContains(log, "SaveEffect: Transition: loadmap [console.map]", "console: accepted effect written")) passed = false;
        if (!logContains(log, "SaveEffect: Unknown command 'badcommand'", "file: rejected effect written")) passed = false;
        if (!logContains(log, "SaveEffect: Transition: loadmap [file.map]", "file: accepted effect written")) passed = false;

        return passed;
    }
   

}

namespace {
// Verify the exact newly appended bytes, not entries left over from previous tests.
bool testExactNotifications(LogObserver* logger) {
    bool passed = true;
    const auto check = [&passed](bool condition, const std::string& label) {
        std::cout << "  " << label << ": " << (condition ? "PASS" : "FAIL") << '\n';
        passed = passed && condition;
    };
    const auto expectEntry = [&check](const std::string& before, const std::string& expected,
                                    const std::string& label) {
        check(readLogFile() == before + expected + "\n", label);
    };

    // A second observer and its copies must not destroy the current game log.
    std::string before = readLogFile();
    LogObserver second;
    LogObserver copy(second);
    LogObserver assigned;
    assigned = second;
    check(readLogFile() == before, "constructing/copying/assigning loggers preserves existing entries");

    Map map;
    Continent* continent = map.addContinent("Demo", 2);
    Territory* one = map.addTerritory(1, "One", continent);
    Territory* two = map.addTerritory(2, "Two", continent);
    Territory* three = map.addTerritory(3, "Three", continent);
    one->addAdjacentTerritory(two); two->addAdjacentTerritory(one);
    one->addAdjacentTerritory(three); three->addAdjacentTerritory(one);
    two->addAdjacentTerritory(three); three->addAdjacentTerritory(two);
    GameEngine engine;
    engine.setupGame(map, {"Alice", "Bob"});
    // Attach after setup as a stronger test of propagation to existing order lists.
    engine.attach(logger);
    engine.attach(logger); // Duplicate attachment must not duplicate entries.
    std::istringstream input("loadmap exact.map\n");
    CommandProcessor processor(input);
    engine.setCommandProcessor(&processor);

    before = readLogFile();
    Command* command = processor.getCommand();
    expectEntry(before, "SaveCommand(): loadmap exact.map", "saveCommand logs exactly one command entry");
    before = readLogFile();
    command->saveEffect("Exact effect");
    expectEntry(before, "SaveEffect: Exact effect", "saveEffect logs exactly the saved effect once");
    before = readLogFile();
    engine.transition("issue orders");
    expectEntry(before, "Game Engine Transition State: issue orders", "transition logs exactly one new-state entry");

    Player* alice = engine.getPlayers()[0];
    Player* bob = engine.getPlayers()[1];
    Territory* target = alice->getTerritoryList()[0];
    alice->setReinforcementPool(3);
    before = readLogFile();
    Deploy* deploy = new Deploy(alice, target, 3);
    alice->getOrdersList()->addOrder(deploy);
    expectEntry(before, "AddOrder(): Order Description: Deploy 3 armies to One.",
                "player list insertion logs the actual configured order exactly once");
    before = readLogFile();
    deploy->execute();
    expectEntry(before, "Order Executed: Deployed 3 armies to One.",
                "valid execute logs the actual stored effect exactly once");
    check(target->getArmyCount() == 3, "logged deployment corresponds to a real army change");

    // Every execute override must notify, even when execution validation rejects it.
    std::vector<Order*> invalidOrders = {new Deploy(), new Advance(), new Bomb(),
                                       new Blockade(), new Airlift(), new Negotiate()};
    const std::vector<std::string> descriptions = {"Deploy Order", "Advance Order", "Bomb Order",
                                                 "Blockade Order", "Airlift Order", "Negotiate Order"};
    for (size_t i = 0; i < invalidOrders.size(); ++i) {
        Order* order = invalidOrders[i];
        alice->getOrdersList()->addOrder(order);
        before = readLogFile();
        order->execute();
        expectEntry(before, "Order Executed: Invalid order: " + descriptions[i],
                    descriptions[i] + " rejection logs one saved execution effect");
    }

    before = readLogFile();
    engine.detach(logger);
    command->saveEffect("Not logged after detach");
    engine.transition("execute orders");
    deploy->execute();
    check(readLogFile() == before, "engine detach removes child command and order registrations");
    engine.attach(logger);

    // Lifetime of processor is shorter than engine; detach the borrowed processor pointer.
    engine.setCommandProcessor(nullptr);
    (void)bob;
    return passed;
}

// Real startup + gameplay path, including order-list wiring and every phase state.
bool testGameplayLogging(LogObserver* logger) {
    bool passed = true;
    const auto check = [&passed](bool value, const std::string& label) {
        std::cout << "  " << label << ": " << (value ? "PASS" : "FAIL") << '\n';
        passed = passed && value;
    };
    const std::string before = readLogFile();
    std::istringstream input("loadmap nyc/NYC.map\nvalidatemap\naddplayer Alice\naddplayer Bob\ngamestart\n");
    CommandProcessor processor(input);
    GameEngine engine;
    engine.attach(logger);
    engine.setCommandProcessor(&processor);
    engine.startupPhase();
    check(engine.getPlayers().size() == 2 && engine.getCurrentStateName() == "assign reinforcement",
          "real startup loads a map and creates both player lists");
    if (engine.getPlayers().size() != 2) return false;
    engine.reinforcementPhase();
    engine.issueOrdersPhase();
    engine.executeOrdersPhase();
    const std::string appended = readLogFile().substr(before.size());
    for (const std::string& text : {"SaveCommand(): loadmap nyc/NYC.map", "SaveEffect: Transition: gamestart",
            "Game Engine Transition State: map loaded", "Game Engine Transition State: map validated",
            "Game Engine Transition State: players added", "Game Engine Transition State: assign reinforcement",
            "Game Engine Transition State: issue orders", "Game Engine Transition State: execute orders",
            "AddOrder(): Order Description: Deploy", "Order Executed: Deployed"})
        check(appended.find(text) != std::string::npos, "real gameplay log contains " + text);

    // Real cleanup is retained by the conflict resolution, and logs exactly once.
    engine.transition("win");
    const std::string replayBefore = readLogFile();
    bool replayed = engine.processCommand(std::string("replay"));
    check(replayed && engine.getPlayers().empty() && engine.getGameMap() == nullptr &&
          readLogFile() == replayBefore + "Game Engine Transition State: start\n",
          "replay preserves Part 3 cleanup and emits one start-state log entry");
    engine.transition("win");
    const std::string quitBefore = readLogFile();
    check(engine.processCommand(std::string("quit")) && engine.isFinished() &&
          readLogFile() == quitBefore + "Game Engine Transition State: end\n",
          "quit cleans up and logs the end transition exactly once");
    return passed;
}
}

bool testLoggingObserver() {

    bool passed = true;

    LogObserver::resetLog();
    LogObserver* logObserver = new LogObserver();
    
    // ------------------------------------------------------------
    // TEST 1: all five classes are a Subject and an ILoggable
    // ------------------------------------------------------------
    std::cout << "\nTEST 1: Inheritance from Subject and ILoggable" << std::endl;
    if (!testInheritance()) {
        passed = false;
    }
    
    // ------------------------------------------------------------
    // TEST 2: copy constructor and assignment operator work for all five classes
    // ------------------------------------------------------------
    std::cout << "\nTEST 2: Copy constructor and assignment operator" << std::endl;
    if (!testCopyAndAssignment(logObserver)) {
        passed = false;
    }
  
    // ------------------------------------------------------------
    // TEST 3: commands, their effects and state changes are logged
    // ------------------------------------------------------------
    std::cout << "\nTEST 3: Commands, effects and game state changes" << std::endl;
    if (!testGameLog(logObserver)) {
        passed = false;
    }

    // ------------------------------------------------------------
    // TEST 4: commands from the console and from a file are logged with their effect
    // (rubric 5.2.11)
    // ------------------------------------------------------------
    std::cout << "\nTEST 4: Commands from the console and from a file" << std::endl;
    if (!testConsoleAndFileCommands(logObserver)) {
        passed = false;
    }
   


    std::cout << "\nTEST 5: Exact notifications and registration propagation\n";
    if (!testExactNotifications(logObserver)) passed = false;
    std::cout << "\nTEST 6: Real startup and Part 3 integration\n";
    if (!testGameplayLogging(logObserver)) passed = false;

    // The observer was created here, so it is deleted here.
    delete logObserver;
    logObserver = nullptr;


    // final summary of the test results
    std::cout << "\nLOGGING OBSERVER TEST RESULTS: " << std::endl;

    //to make function return if the tess pass or not
    const std::string log = readLogFile();

    //expected entries in the log file
    const std::string expectedEntries[] = {
        "SaveCommand(): gamestart",
        "SaveCommand(): loadmap canada.map",
        "SaveCommand(): validatemap",
        "SaveCommand(): addplayer Alice",
        "SaveCommand(): addplayer Bob",
        "SaveCommand(): wrongCommand",
        "SaveEffect: Command 'gamestart' is not allowed",
        "SaveEffect: Unknown command 'wrongCommand'",
        "SaveEffect: Transition: loadmap [canada.map]",
        "AddOrder()",
        "Order Executed:",
        "Game Engine Transition State: map loaded"
    };

  
    //check if all expected entries are in the log file
    for (const std::string& entry : expectedEntries) {
        if (!logContains(log, entry, entry)) {
            passed = false;
        }
    }


    std::cout << "\nLOGGING OBSERVER DRIVER: " << (passed ? "PASS" : "FAIL") << std::endl;


    return passed;

    
}
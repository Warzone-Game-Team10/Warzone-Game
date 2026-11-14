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
        // (only the default constructor does that).
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

bool testLoggingObserver() {

    bool passed = true;

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
#include "CommandProcessingDriver.h"
#include "CommandProcessing.h"
#include "GameEngine.h"
#include "GameEngineDriver.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <filesystem>
#include <fstream>

namespace {
/**
 * Describes a named scenario and records whether it passed, failed, or was not run
 */
struct TestScenario {
    std::string section;
    std::string name;
    std::string expected;
    void (*run)();
    std::string result = "NOT RUN";
};

// makes exact input text readable, including whitespace and control characters
std::string quotedInput(const std::string& text) {
    std::string quoted = "\"";
    for (char character : text) {
        switch (character) {
            case '\n': quoted += "\\n"; break;
            case '\r': quoted += "\\r"; break;
            case '\t': quoted += "\\t"; break;
            case '\\': quoted += "\\\\"; break;
            case '"': quoted += "\\\""; break;
            default: quoted += character;
        }
    }
    return quoted + "\"";
}

// prints a labelled input or observed value
void printInput(const std::string& label, const std::string& text) {
    std::cout << "  " << label << ": " << quotedInput(text) << '\n';
}

// shows an input, its validation result, and its effect without claiming execution
bool validateWithTrace(CommandProcessor& processor, Command* command, const std::string& state) {
    const bool valid = processor.validate(command, state);
    std::cout << "  " << quotedInput(command->getText()) << " @ " << state
              << " -> " << (valid ? "ACCEPT" : "REJECT")
              << "\n    Effect: " << quotedInput(command->getEffect()) << "\n\n";
    return valid;
}

// runs the shared driver and displays each newly processed command exactly once
void traceDriver(GameEngine& engine, CommandProcessor& processor, std::ostringstream& output,
                 const std::string& label = "Execution") {
    const auto firstCommand = processor.getCommandCount();
    std::cout << "  " << label << " from " << engine.getCurrentStateName() << ":\n";
    runCommandTransitions(engine, processor, output);
    for (std::size_t i = firstCommand; i < processor.getCommandCount(); ++i) {
        const Command& command = *processor.getSavedCommand(i);
        std::cout << "    Input:  " << quotedInput(command.getText())
                  << "\n    Effect: " << command.getEffect() << "\n\n";
    }
    std::cout << "  End: " << engine.getCurrentStateName()
              << "; history=" << processor.getCommandCount()
              << "; " << (engine.isFinished() ? "quit accepted" : "EOF") << "\n\n";
}

// throws a test failure with the given message when a condition is false
void check(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("Command processing check failed: " + message);
    }
}

// runs a named test and reports success only after all its checks pass
void runTest(TestScenario& scenario, std::size_t number) {
    std::cout << "\n[" << number << "] " << scenario.name
              << "\n\n  Expected:\n    " << scenario.expected << "\n\n  Actual results:\n\n";
    scenario.result = "FAIL";
    try {
        scenario.run();
    } catch (const std::exception& error) {
        std::cout << "\n  Result: FAIL - expectations not met.\n"
                  << "  Reason: " << error.what() << '\n';
        throw;
    } catch (...) {
        std::cout << "\n  Result: FAIL - expectations could not be verified.\n"
                  << "  Reason: an exception without a diagnostic message interrupted the scenario.\n";
        throw;
    }
    scenario.result = "PASS";
    std::cout << "\n  Result: PASS - all expectations met.\n";
}

// lists every scenario's result, including those skipped after a failure
void printTestSummary(const std::vector<TestScenario>& scenarios) {
    std::cout << "\n========== RESULTS ==========\n\n";
    std::size_t passed = 0;
    std::size_t failed = 0;
    for (std::size_t i = 0; i < scenarios.size(); ++i) {
        const auto& scenario = scenarios[i];
        std::cout << "  " << std::left << std::setw(9) << scenario.result
                  << '[' << i + 1 << "] " << scenario.name << '\n';
        if (scenario.result == "PASS") ++passed;
        if (scenario.result == "FAIL") ++failed;
    }
    const auto notRun = scenarios.size() - passed - failed;
    std::cout << "\nCOMMAND PROCESSING SUMMARY: " << passed << '/' << scenarios.size()
              << " scenarios passed; " << failed << " failed; " << notRun << " not run.\n";
    if (notRun != 0) {
        std::cout << "Stopped at first failure; later scenarios were not run.\n";
    }
}

// checks each command against allowed and rejected states and verifies its effect
void testCommandStateRules() {
    const std::vector<std::string> states = {
        "start", "map loaded", "map validated", "players added", "assign reinforcement",
        "issue orders", "execute orders", "win", "end", "unknown"
    };

    /**
     * Stores a command and the states where it is allowed
     */
    struct Scenario {
        std::string text;
        std::vector<std::string> allowedStates;
    };
    const std::vector<Scenario> scenarios = {
        {"loadmap maps/my map.map", {"start", "map loaded"}},
        {"validatemap", {"map loaded"}},
        {"addplayer John Smith", {"map validated", "players added"}},
        {"gamestart", {"players added"}},
        {"replay", {"win"}},
        {"quit", {"win"}}
    };
    std::istringstream unused;
    CommandProcessor processor(unused);

    std::cout << "  A=accept (effect unchanged), R=reject (command/state error).\n";
    for (std::size_t i = 0; i < states.size(); ++i) {
        std::cout << "  " << i << '=' << states[i] << (i == 4 || i == 9 ? "\n" : ";");
    }
    std::cout << "\n  " << std::left << std::setw(34) << "Input / state";
    for (std::size_t i = 0; i < states.size(); ++i) std::cout << std::setw(3) << i;
    std::cout << "\n  " << std::string(64, '-') << '\n';
    for (const auto& scenario : scenarios) {
        std::cout << "  " << std::setw(34) << quotedInput(scenario.text);
        for (const auto& state : states) {
            bool expected = false;
            for (const auto& allowed : scenario.allowedStates) {
                if (state == allowed) expected = true;
            }
            Command command(scenario.text);
            command.saveEffect("Previous execution outcome");
            const bool valid = processor.validate(&command, state);
            std::cout << std::setw(3) << (valid ? 'A' : 'R');
            check(valid == expected,
                  scenario.text + " in " + state + ": expected " + (expected ? "ACCEPT" : "REJECT") +
                  ", got " + (valid ? "ACCEPT" : "REJECT"));
            check(command.getText() == scenario.text, "validation preserves original text");
            if (expected) {
                check(command.getEffect() == "Previous execution outcome",
                      "acceptance does not claim execution or overwrite its effect");
            } else {
                check(command.getEffect().find(state) != std::string::npos &&
                      command.getEffect().find(command.getName()) != std::string::npos,
                      "state rejection identifies command and state");
            }
        }
        std::cout << '\n';
    }
}

// checks rejection effects for empty input, unknown commands, and invalid arguments
void testInvalidCommandSyntax() {
    std::istringstream unused;
    CommandProcessor processor(unused);

    /**
     * Stores malformed input, a test state, and the expected error text
     */
    struct InvalidSyntax {
        std::string text;
        std::string state;
        std::string error;
    };
    const InvalidSyntax malformed[] = {
        {"", "start", "empty"}, {" \t\r", "start", "empty"},
        {"LOADMAP map", "start", "Unknown"}, {"unknown", "start", "Unknown"},
        {"assigncountries", "players added", "Unknown"}, {"play", "win", "Unknown"},
        {"end", "win", "Unknown"},
        {"loadmap", "start", "argument is required"}, {"loadmap \t", "map loaded", "argument is required"},
        {"addplayer", "map validated", "argument is required"},
        {"addplayer \r\t", "players added", "argument is required"},
        {"validatemap extra", "map loaded", "no argument is allowed"},
        {"gamestart extra", "players added", "no argument is allowed"},
        {"replay extra", "win", "no argument is allowed"}, {"quit extra", "win", "no argument is allowed"}
    };

    std::cout << "  Malformed inputs (each also tested in unknown):\n\n";
    for (const auto& scenario : malformed) {
        for (const auto& state : {scenario.state, std::string("unknown")}) {
            Command command(scenario.text);
            command.saveEffect("Old effect");
            const bool valid = processor.validate(&command, state);
            if (state == scenario.state) {
                std::cout << "  " << quotedInput(scenario.text) << " @ " << state
                          << " + unknown\n    " << (valid ? "ACCEPT: " : "REJECT: ")
                          << command.getEffect() << "\n\n";
            }
            check(!valid, "reject malformed input: " + scenario.text);
            check(command.getEffect().find(scenario.error) != std::string::npos,
                  "syntax checked before state, with a descriptive replacement effect: " + scenario.text);
        }
    }
}

// checks whitespace handling and preserves quotes and other literal argument characters
void testCommandWhitespaceAndLiterals() {
    std::cout << "  Whitespace and literal arguments:\n\n";
    std::istringstream unused;
    CommandProcessor processor(unused);
    Command spaced(" \tloadmap \"my map\" #literal \r");
    check(validateWithTrace(processor, &spaced, "start") && spaced.getEffect().empty() &&
          spaced.getArgument() == "\"my map\" #literal", "whitespace and literal arguments accepted");
    Command noArgument(" \tquit \r");
    check(validateWithTrace(processor, &noArgument, "win"), "trailing whitespace is not an argument");
}

// checks that reading continues after rejected commands and retains their effects
void testReadingAfterCommandRejection() {
    std::cout << "  Continue reading after rejection:\n\n";
    std::istringstream input("\nloadmap\nquit\nloadmap maps/my map.map\n");
    CommandProcessor captured(input);
    const std::string state = "start";
    int accepted = 0;
    while (Command* command = captured.getCommand()) {
        if (validateWithTrace(captured, command, state)) ++accepted;
    }
    check(accepted == 1 && captured.getCommandCount() == 4 && state == "start",
          "rejections preserve state and reading continues to valid input and EOF");
    for (std::size_t i = 0; i < 3; ++i) {
        check(!captured.getSavedCommand(i)->getEffect().empty(), "history retains rejection effects");
    }
    check(captured.getSavedCommand(3)->getEffect().empty(), "accepted capture has no execution effect");
}

// moves the engine to win through internal transitions for replay and quit tests
void reachWin(GameEngine& engine) {
    std::cout << "  Setup: win via internal engine events (not user input).\n";
    for (const std::string event : {"loadmap", "validatemap", "addplayer", "gamestart",
                                    "issueorder", "endissueorders", "win"}) {
        check(engine.processCommand(event), "win fixture transition: " + event);
    }
}

// checks startup transitions, self-loops, and effects stored in command history
void testStoredCommandTransitions() {

    /**
     * Stores a command and its expected destination state
     */
    struct TransitionScenario {
        std::string text;
        std::string expectedState;
    };
    const TransitionScenario scenarios[] = {
        {" \tloadmap maps/my map.map ", "map loaded"},
        {"loadmap other.map", "map loaded"},
        {"validatemap", "map validated"},
        {"addplayer John Smith", "players added"},
        {"addplayer Jane", "players added"},
        {"gamestart", "assign reinforcement"}
    };
    std::ostringstream commandLines;
    for (const auto& scenario : scenarios) {
        commandLines << scenario.text << '\n';
    }
    std::istringstream input(commandLines.str());
    CommandProcessor processor(input);
    GameEngine engine;
    for (const auto& scenario : scenarios) {
        const std::size_t historyIndex = processor.getCommandCount();
        Command* command = processor.getCommand();
        check(command && processor.validate(command, engine.getCurrentStateName()), "valid stored input");
        const std::string before = engine.getCurrentStateName();
        check(engine.processCommand(*command), "stored command executes");
        std::cout << "  Input:  " << quotedInput(command->getText())
                  << "\n  Effect: " << command->getEffect() << "\n\n";
        check(engine.getCurrentStateName() == scenario.expectedState, "expected transition destination");
        check(processor.getSavedCommand(historyIndex) == command &&
              command->getEffect().find("Transition: ") != std::string::npos &&
              command->getEffect().find(before) != std::string::npos &&
              command->getEffect().find(scenario.expectedState) != std::string::npos,
              "original history records transition outcome");
        if (!command->getArgument().empty()) {
            check(command->getEffect().find(command->getArgument()) != std::string::npos,
                  "effect identifies parsed argument");
        }
    }
    check(processor.getSavedCommand(0)->getText() == " \tloadmap maps/my map.map ",
          "execution preserves original text");
    std::ostringstream output;
    traceDriver(engine, processor, output);
    check(engine.getCurrentStateName() == "assign reinforcement" && !engine.isFinished() &&
          processor.getCommandCount() == 6 &&
          output.str().find("End of input. Command processing stopped.") != std::string::npos,
          "EOF after gamestart preserves state and reports stopped input");
    Command unavailable("loadmap missing.map");
    printInput("Direct engine input (without processor validation)", unavailable.getText());
    std::cout << "    State before execution: " << engine.getCurrentStateName() << '\n';
    check(!engine.processCommand(unavailable) &&
          unavailable.getEffect().find("assign reinforcement") != std::string::npos &&
          engine.getCurrentStateName() == "assign reinforcement", "engine failure records unchanged state");
    std::cout << "    State after execution: " << engine.getCurrentStateName() << '\n';
    printInput("Execution effect", unavailable.getEffect());
}

// checks driver rejection effects and verifies that eof preserves state and history
void testTransitionDriverRejectionAndEof() {
    std::istringstream input("\nloadmap\nquit\nloadmap my map.map\nwin\nvalidatemap");
    CommandProcessor processor(input);
    GameEngine engine;
    std::ostringstream output;
    traceDriver(engine, processor, output);
    check(engine.getCurrentStateName() == "map validated" && !engine.isFinished(),
          "rejections preserve state and EOF does not quit");
    check(processor.getCommandCount() == 6, "EOF does not create a command");
    for (std::size_t i : {0, 1, 2, 4}) {
        check(!processor.getSavedCommand(i)->getEffect().empty() &&
              processor.getSavedCommand(i)->getEffect().find("Transition: ") == std::string::npos,
              "rejection effect survives driver processing");
    }
    check(output.str().find("End of input. Command processing stopped.") != std::string::npos &&
          output.str().find("Game finished") == std::string::npos, "incomplete EOF report");
    const std::string effect = processor.getSavedCommand(5)->getEffect();
    traceDriver(engine, processor, output, "Repeated EOF read");
    check(processor.getCommandCount() == 6 && processor.getSavedCommand(5)->getEffect() == effect,
          "repeated EOF preserves history and effects");
    std::istringstream empty;
    CommandProcessor emptyProcessor(empty);
    GameEngine fresh;
    traceDriver(fresh, emptyProcessor, output, "Empty stream execution");
    check(fresh.getCurrentStateName() == "start" && emptyProcessor.getCommandCount() == 0,
          "immediate EOF preserves initial state");
}

// checks that replay returns to start and the driver continues reading commands
void testTransitionDriverReplay() {
    GameEngine engine;
    reachWin(engine);
    std::istringstream replayInput("replay\nloadmap replay map.map\n");
    CommandProcessor replayProcessor(replayInput);
    std::ostringstream output;
    traceDriver(engine, replayProcessor, output, "Console");
    check(engine.getCurrentStateName() == "map loaded" &&
          replayProcessor.getSavedCommand(0)->getEffect().find("'win' -> 'start'") != std::string::npos,
          "replay returns to start and continues reading");
}

// checks that quit ends the driver and leaves later commands unread
void testTransitionDriverQuit() {
    GameEngine quitting;
    reachWin(quitting);
    std::istringstream quitInput("quit\nloadmap unread.map\n");
    CommandProcessor quitProcessor(quitInput);
    std::ostringstream output;
    traceDriver(quitting, quitProcessor, output, "Console");
    check(quitting.isFinished() && quitProcessor.getCommandCount() == 1 &&
          quitProcessor.getSavedCommand(0)->getEffect().find("'win' -> 'end'") != std::string::npos &&
          output.str().find("Quit accepted") != std::string::npos &&
          output.str().find("End of input") == std::string::npos, "quit ends run before another read");
    Command* unread = quitProcessor.getCommand();
    check(unread && unread->getText() == "loadmap unread.map", "quit leaves trailing input unread");
    printInput("Unread trailing command", unread->getText());
}

/**
 * Creates a temporary command file and removes its directory after the test
 */
struct CommandFileFixture {
    std::filesystem::path directory;
    std::filesystem::path file;
    explicit CommandFileFixture(const std::string& contents); // parameterized constructor
    ~CommandFileFixture(); // destructor
};

CommandFileFixture::CommandFileFixture(const std::string& contents) { // parameterized constructor
    directory = std::filesystem::temp_directory_path() /
                ("warzone-commands-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    check(std::filesystem::create_directory(directory), "create isolated command fixture directory");
    file = directory / "commands.txt";
    std::ofstream output(file, std::ios::binary);
    output << contents;
    output.close();
    if (!output) {
        std::filesystem::remove_all(directory);
        throw std::runtime_error("Cannot write command fixture.");
    }
}

CommandFileFixture::~CommandFileFixture() { // destructor
    std::error_code error;
    std::filesystem::remove_all(directory, error);
}

// checks file line order, blank lines, eof, and independent reader copies
void testFileReaderLinesAndCopies() {
    CommandFileFixture fixture("first\n\nlast");
    printInput("File input", "first\n\nlast");
    FileLineReader reader(fixture.file.string());
    std::string line;
    check(reader.readLineFromFile(line) && line == "first", "file starts at first line");
    printInput("Source reader line 1", line);
    std::cout << "    Action: copy and assign the reader at its current position\n";
    FileLineReader copied(reader);
    FileLineReader assigned(fixture.file.string());
    assigned = reader;
    const FileLineReader& same = assigned;
    assigned = same;
    check(reader.readLineFromFile(line) && line.empty(), "blank file line retained");
    printInput("Source reader line 2", line);
    check(reader.readLineFromFile(line) && line == "last", "last line without newline retained");
    printInput("Source reader line 3 (no final newline)", line);
    FileLineReader finalLineCopy(reader);
    check(!finalLineCopy.readLineFromFile(line), "copy after unterminated last line preserves EOF");
    check(!reader.readLineFromFile(line) && !reader.readLineFromFile(line), "repeated file EOF");
    FileLineReader exhausted(reader);
    check(!exhausted.readLineFromFile(line), "exhausted copy stays exhausted");
    for (FileLineReader* independent : {&copied, &assigned}) {
        check(independent->readLineFromFile(line) && line.empty(), "copy has independent same-position cursor");
        printInput("Independent copy next line", line);
        check(independent->readLineFromFile(line) && line == "last", "copy retains remaining order");
        printInput("Independent copy final line", line);
    }
    std::ostringstream output;
    output << reader;
    check(output.str().find("commands.txt") != std::string::npos, "reader stream output identifies file");
    printInput("Empty file", "");
    CommandFileFixture empty("");
    FileLineReader emptyReader(empty.file.string());
    check(!emptyReader.readLineFromFile(line), "empty file immediately exhausts");
    printInput("Terminated file", "first\nsecond\n");
    CommandFileFixture terminated("first\nsecond\n");
    FileLineReader terminatedReader(terminated.file.string());
    check(terminatedReader.readLineFromFile(line) && line == "first", "newline-terminated input");
    {
        FileLineReader temporary(terminatedReader);
        assigned = temporary;
    }
    check(assigned.readLineFromFile(line) && line == "second", "assigned reader survives source destruction");
    check(!assigned.readLineFromFile(line), "trailing newline creates no extra line");
}

// checks that file and console input produce matching states, output, and histories
void testFileAdapterMatchesConsole() {
    const std::string text = "\nloadmap\nloadmap my map.map\nquit\nvalidatemap\naddplayer John Smith\ngamestart";
    CommandFileFixture fixture(text);
    FileCommandProcessorAdapter file(fixture.file.string());
    std::istringstream input(text);
    CommandProcessor console(input);
    GameEngine fileEngine;
    GameEngine consoleEngine;
    std::ostringstream fileOutput;
    std::ostringstream consoleOutput;
    traceDriver(fileEngine, file, fileOutput, "File execution");
    traceDriver(consoleEngine, console, consoleOutput, "Console execution");
    check(fileOutput.str() == consoleOutput.str() && fileEngine.getCurrentStateName() == "assign reinforcement",
          "console and file share validation, effects, transitions, and EOF");
    check(file.getCommandCount() == 7, "file history retains accepted and rejected lines");
    for (std::size_t i = 0; i < file.getCommandCount(); ++i) {
        check(file.getSavedCommand(i)->getText() == console.getSavedCommand(i)->getText() &&
              file.getSavedCommand(i)->getEffect() == console.getSavedCommand(i)->getEffect(),
              "console and file histories agree");
    }
    FileCommandProcessorAdapter exhausted(file);
    check(!exhausted.getCommand(), "adapter copy preserves EOF");
}

// checks independent adapter histories and readers, copying, and input through a base pointer
void testFileAdapterCopies() {
    CommandFileFixture fixture("\nloadmap\nloadmap my map.map\n");
    printInput("Copy fixture", "\nloadmap\nloadmap my map.map\n");
    FileCommandProcessorAdapter source(fixture.file.string());
    source.getCommand()->saveEffect("saved effect");
    FileCommandProcessorAdapter copied(source);
    FileCommandProcessorAdapter assigned(fixture.file.string());
    assigned.getCommand();
    assigned.getCommand();
    assigned = source;
    const FileCommandProcessorAdapter& same = assigned;
    assigned = same;
    copied.getSavedCommand(0)->saveEffect("copy effect");
    std::cout << "  Adapter effects: source=" << quotedInput(source.getSavedCommand(0)->getEffect())
              << "; copy=" << quotedInput(copied.getSavedCommand(0)->getEffect())
              << "; assigned=" << quotedInput(assigned.getSavedCommand(0)->getEffect()) << '\n';
    check(source.getSavedCommand(0)->getEffect() == "saved effect" &&
          assigned.getSavedCommand(0)->getEffect() == "saved effect" &&
          copied.getSavedCommand(0) != source.getSavedCommand(0), "adapter histories and effects are independent");
    std::cout << "    Action: advance source twice; each copy should still read loadmap next\n";
    check(source.getCommand()->getText() == "loadmap", "source reads its next command");
    check(source.getCommand()->getText() == "loadmap my map.map", "source advances independently");
    check(copied.getCommand()->getText() == "loadmap", "constructed copy retains its own cursor");
    check(assigned.getCommand()->getText() == "loadmap", "assigned copy retains its own cursor");
    {
        FileCommandProcessorAdapter temporary(fixture.file.string());
        temporary.getCommand();
        assigned = temporary;
    }
    check(assigned.getCommand()->getText() == "loadmap", "assigned adapter survives source destruction");
    std::ostringstream output;
    output << copied;
    check(output.str().find("commands.txt") != std::string::npos &&
          output.str().find("copy effect") != std::string::npos, "adapter stream output shows file and history");
    std::unique_ptr<CommandProcessor> polymorphic = std::make_unique<FileCommandProcessorAdapter>(fixture.file.string());
    check(polymorphic->getCommand()->getText().empty(), "base interface dispatches file input");
}

// checks that replay from file input returns to start and continues reading
void testFileAdapterReplay() {
    CommandFileFixture fixture("replay\nloadmap next.map\n");
    FileCommandProcessorAdapter processor(fixture.file.string());
    GameEngine engine;
    reachWin(engine);
    std::ostringstream output;
    traceDriver(engine, processor, output, "File");
    check(engine.getCurrentStateName() == "map loaded" && processor.getCommandCount() == 2,
          "file replay continues from start");
}

// checks that quit from file input ends processing and leaves later commands unread
void testFileAdapterQuit() {
    CommandFileFixture fixture("quit\nloadmap next.map\n");
    FileCommandProcessorAdapter processor(fixture.file.string());
    GameEngine engine;
    reachWin(engine);
    std::ostringstream output;
    traceDriver(engine, processor, output, "File");
    check(engine.isFinished() && processor.getCommandCount() == 1, "file quit stops reading");
    Command* unread = processor.getCommand();
    check(unread && unread->getText() == "loadmap next.map", "file quit leaves trailing input unread");
    printInput("Unread trailing command", unread->getText());
}

// checks file errors and verifies failed assignments preserve existing history and position
void testFileFailures() {
    printInput("File input", "first\nsecond\n");
    CommandFileFixture fixture("first\nsecond\n");
    bool threw = false;
    try { FileCommandProcessorAdapter missing((fixture.directory / "missing.txt").string()); }
    catch (const std::runtime_error& error) {
        printInput("Caught error", error.what());
        threw = std::string(error.what()).find("missing.txt") != std::string::npos;
    }
    check(threw, "missing file reports its path");
    FileLineReader source(fixture.file.string());
    FileLineReader destination(fixture.file.string());
    std::string line;
    check(destination.readLineFromFile(line), "advance destination before failed assignment");
    printInput("Destination reader consumed", line);
    FileCommandProcessorAdapter adapterSource(fixture.file.string());
    FileCommandProcessorAdapter adapterDestination(fixture.file.string());
    adapterDestination.getCommand()->saveEffect("preserved");
    std::cout << "    Action: rename the open command file to moved.txt before copy/assignment attempts\n";
    std::filesystem::rename(fixture.file, fixture.directory / "moved.txt");
    threw = false;
    try { FileLineReader copy(source); } catch (const std::runtime_error& error) {
        printInput("Caught error", error.what());
        threw = true;
    }
    check(threw, "reader copy reports reopen failure");
    threw = false;
    try { destination = source; } catch (const std::runtime_error& error) {
        printInput("Caught error", error.what());
        threw = true;
    }
    check(threw && destination.readLineFromFile(line) && line == "second", "failed reader assignment preserves cursor");
    printInput("Destination reader next line after failed assignment", line);
    threw = false;
    try { adapterDestination = adapterSource; } catch (const std::runtime_error& error) {
        printInput("Caught error", error.what());
        threw = true;
    }
    check(threw && adapterDestination.getCommandCount() == 1 &&
          adapterDestination.getSavedCommand(0)->getEffect() == "preserved" &&
          adapterDestination.getCommand()->getText() == "second", "failed adapter assignment preserves history and cursor");
    threw = false;
    try {
        FileCommandProcessorAdapter directory(fixture.directory.string());
        directory.getCommand();
    } catch (const std::runtime_error& error) {
        printInput("Caught error", error.what());
        threw = true;
    }
    check(threw, "directory input reports an open or read error instead of EOF");
}

// checks command capture, effect copying, history ownership, and stream boundaries
void testCommandCapture() {
    std::cout << "  Capture and command copying:\n";
    std::istringstream input(" \tloadmap  maps/my map.map \t\n\n \t\naddplayer John Smith");
    printInput("Stream input", input.str());
    CommandProcessor processor(input);
    Command* first = processor.getCommand();
    check(first && first->getText() == " \tloadmap  maps/my map.map \t", "original text preserved");
    check(first->getName() == "loadmap" && first->getArgument() == "maps/my map.map", "parsed views trim edges");
    check(first->getEffect().empty(), "capture does not claim execution");
    first->saveEffect("Map unavailable");
    printInput("Saved effect", first->getEffect());
    check(processor.getSavedCommand(0)->getEffect() == "Map unavailable", "effect retained in history");
    Command valueCopy(*first);
    Command assigned("quit");
    printInput("Assignment destination input", assigned.getText());
    assigned = *first;
    first->saveEffect("Changed");
    printInput("Original effect after change", first->getEffect());
    printInput("Copy-constructed effect", valueCopy.getEffect());
    printInput("Copy-assigned effect", assigned.getEffect());
    check(valueCopy.getEffect() == "Map unavailable" && assigned.getEffect() == "Map unavailable", "command copies are independent");
    const Command& same = assigned;
    assigned = same;
    check(assigned.getText() == valueCopy.getText(), "command self-assignment preserves text");
    check(processor.getCommand()->getText().empty(), "blank line is a command");
    Command* whitespace = processor.getCommand();
    check(whitespace->getText() == " \t" && whitespace->getName().empty(), "whitespace preserved");
    Command* last = processor.getCommand();
    check(last && last->getArgument() == "John Smith", "last unterminated line captured");
    check(!processor.getCommand() && !processor.getCommand() && processor.getCommandCount() == 4, "EOF creates no commands");
    check(first == processor.getSavedCommand(0), "borrowed pointer survives history growth");
    std::cout << "  Captured 4 commands; repeated EOF adds none; original pointer retained.\n";

    std::cout << "\n  Processor history copying:\n";
    CommandProcessor copied(processor);
    std::istringstream otherInput("old\n");
    printInput("Stream input (otherInput)", otherInput.str());
    CommandProcessor replaced(otherInput);
    replaced.getCommand();
    replaced = processor;
    const CommandProcessor& sameProcessor = replaced;
    replaced = sameProcessor;
    for (std::size_t i = 0; i < processor.getCommandCount(); ++i) {
        check(copied.getSavedCommand(i) != processor.getSavedCommand(i) &&
              replaced.getSavedCommand(i) != processor.getSavedCommand(i), "history copies own distinct commands");
        check(copied.getSavedCommand(i)->getText() == processor.getSavedCommand(i)->getText(), "copied history preserves order and text");
    }
    copied.getSavedCommand(0)->saveEffect("Copy changed");
    std::cout << "  History effects: source=" << quotedInput(first->getEffect())
              << "; copy=" << quotedInput(copied.getSavedCommand(0)->getEffect())
              << "; assigned=" << quotedInput(replaced.getSavedCommand(0)->getEffect()) << '\n';
    check(first->getEffect() == "Changed" && replaced.getSavedCommand(0)->getEffect() == "Changed", "copied effects independent");
    {
        CommandProcessor temporary(processor);
        replaced = temporary;
    }
    check(replaced.getSavedCommand(0)->getEffect() == "Changed", "history survives source destruction");
    std::ostringstream rendered;
    rendered << valueCopy << copied;
    check(rendered.str().find("Map unavailable") != std::string::npos && rendered.str().find("Copy changed") != std::string::npos, "stream operators show effects");
    std::istringstream emptyInput;
    CommandProcessor empty(emptyInput);
    replaced = empty;
    check(replaced.getCommandCount() == 0 && !replaced.getCommand(), "empty assignment clears history");

    std::cout << "\n  Stream failures and EOF:\n";
    for (auto state : {std::ios::failbit, std::ios::badbit, std::ios::badbit | std::ios::eofbit}) {
        std::cout << "  Injected stream flags:"
                  << (state & std::ios::failbit ? " failbit" : "")
                  << (state & std::ios::badbit ? " badbit" : "")
                  << (state & std::ios::eofbit ? " eofbit" : "") << "\n";
        std::istringstream broken;
        broken.setstate(state);
        CommandProcessor failed(broken);
        bool threw = false;
        try { failed.getCommand(); } catch (const std::runtime_error& error) {
            printInput("Caught error", error.what());
            threw = true;
        }
        check(threw && failed.getCommandCount() == 0, "input failure throws without synthetic command");
    }
    std::istringstream masked("quit");
    printInput("Stream input (masked)", masked.str());
    masked.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);
    CommandProcessor maskedProcessor(masked);
    check(maskedProcessor.getCommand()->getText() == "quit", "exception-enabled final line captured");
    check(!maskedProcessor.getCommand(), "exception-enabled EOF is normal");
    std::cout << "\n  Shared input stream and literal parsing:\n";
    std::istringstream shared("first\nsecond\n");
    printInput("Stream input (shared)", shared.str());
    CommandProcessor source(shared);
    CommandProcessor sharedCopy(source);
    check(source.getCommand()->getText() == "first" && sharedCopy.getCommand()->getText() == "second",
          "processor copies borrow the same input cursor");
    check(source.getCommandCount() == 1 && sharedCopy.getCommandCount() == 1, "shared input has independent histories");
    Command literal("  LOADMAP \"my map\" #literal  ");
    printInput("Literal command input", literal.getText());
    printInput("Parsed name", literal.getName());
    printInput("Parsed argument", literal.getArgument());
    check(literal.getName() == "LOADMAP" && literal.getArgument() == "\"my map\" #literal", "case and literal characters preserved");
}

// runs command transitions with the given processor and prints the saved history
void demonstrateCommands(CommandProcessor& processor) {
    GameEngine engine;
    runCommandTransitions(engine, processor, std::cout);
    std::cout << processor;
}
}

/**
 * testCommandProcessing() runs the command-processing checks.
 * It also checks captured text, effects, copying, stream output, and input failures
 */
bool testCommandProcessing() {
    std::vector<TestScenario> scenarios = {
        {"1. COMMAND VALIDATION", "Command/state matrix: all six commands in ten states",
                "60 validations: 8 accept, 52 reject.\n"
                "    loadmap: start/map loaded; validatemap: map loaded; addplayer: map validated/players added.\n"
                "    gamestart: players added; replay/quit: win. Reject all other states; preserve accepted effects.", testCommandStateRules},
        {"", "Syntax, literal arguments, and continued reading",
                "Reject 15 malformed inputs in both the listed state and unknown, with descriptive syntax errors.\n"
                "    Preserve literal arguments; accept surrounding whitespace. Read past 3 rejections to 1 valid command.", [] {
            testInvalidCommandSyntax();
            testCommandWhitespaceAndLiterals();
            testReadingAfterCommandRejection();
        }},
        {"2. STATE TRANSITIONS AND DRIVER BEHAVIOR", "Startup transitions, self-loops, and stored effects",
                "6 accepted commands reach assign reinforcement, including loadmap/addplayer self-loops.\n"
                "    History stores transition effects and arguments. EOF and unavailable engine transitions preserve state.", testStoredCommandTransitions},
        {"", "Driver rejection, empty input, and repeated EOF",
                "Retain all 6 inputs: 2 transitions and 4 rejections; finish at map validated without quitting.\n"
                "    Repeated EOF preserves history/effects; empty input leaves start with 0 commands.", testTransitionDriverRejectionAndEof},
        {"", "Replay from win: console and file",
                "For both sources, replay changes win to start; the next loadmap reaches map loaded before EOF.", [] {
            testTransitionDriverReplay();
            testFileAdapterReplay();
        }},
        {"", "Quit from win: console and file; trailing input unread",
                "For both sources, quit changes win to end; history contains only quit and the next input remains unread.", [] {
            testTransitionDriverQuit();
            testFileAdapterQuit();
        }},
        {"3. COMMAND CAPTURE AND HISTORY", "Capture, history ownership, copies, and stream errors",
                "Capture 4 exact lines; trim parsed views only. EOF adds no commands.\n"
                "    Command/history copies own independent effects and survive source destruction; stream output shows effects.\n"
                "    Input failures throw without saving commands; normal EOF stays normal with stream exceptions enabled.\n"
                "    Processor copies share the input cursor but own separate histories; parsing preserves case and literals.", testCommandCapture},
        {"4. FILE INPUT AND ADAPTER", "File lines, EOF, and independent reader copies",
                "Read first, a blank line, then last without a final newline; copied readers keep independent cursors.\n"
                "    Preserve EOF when copying; empty files have no lines; trailing newlines add no extra line.\n"
                "    Assigned readers survive source destruction; stream output identifies the file.", testFileReaderLinesAndCopies},
        {"", "Adapter equivalence, independent copies, and base dispatch",
                "Console and file produce identical output/history for 7 inputs, ending at assign reinforcement.\n"
                "    Adapter copies preserve independent histories/cursors, survive source destruction, and retain EOF.\n"
                "    Stream output shows file/history; reading through a base pointer dispatches to the file adapter.", [] {
            testFileAdapterMatchesConsole();
            testFileAdapterCopies();
        }},
        {"", "File errors and failed assignment guarantees",
                "Missing files, directory input, and reopening a renamed file throw descriptive errors.\n"
                "    If assignment fails because the source file was renamed, the destination keeps its saved commands and effects.\n"
                "    Its reading position also stays unchanged: the next line read must still be \"second\".", testFileFailures}
    };
    std::cout << "========================================\n"
              << "COMMAND PROCESSOR AND FILE ADAPTER TESTS\n"
              << "========================================\n"
              << "\nNote: Escaped inputs show exact whitespace.\n";
    try {
        for (std::size_t i = 0; i < scenarios.size(); ++i) {
            if (!scenarios[i].section.empty()) {
                std::cout << "\n========== " << scenarios[i].section << " ==========\n";
            }
            runTest(scenarios[i], i + 1);
        }
    } catch (...) {
        printTestSummary(scenarios);
        throw;
    }
    printTestSummary(scenarios);
    return true;
}

// demonstrates console commands using the shared transition and history output
void demonstrateConsoleCommands() {
    CommandProcessor processor;
    demonstrateCommands(processor);
}

// demonstrates saved file commands using the shared transition and history output
void demonstrateFileCommands(const std::string& filename) {
    FileCommandProcessorAdapter processor(filename);
    demonstrateCommands(processor);
}

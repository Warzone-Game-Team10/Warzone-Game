#include "CommandProcessingDriver.h"
#include "CommandProcessing.h"
#include "GameEngine.h"
#include "GameEngineDriver.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <filesystem>
#include <fstream>

namespace {
void check(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("Command processing check failed: " + message);
    }
}

void testCommandStateRules() {
    const std::vector<std::string> states = {
        "start", "map loaded", "map validated", "players added", "assign reinforcement",
        "issue orders", "execute orders", "win", "end", "unknown"
    };
    struct Scenario {
        std::string text;
        std::vector<std::string> allowedStates;
    };
    const std::vector<Scenario> scenarios = {
        {"loadmap maps/my map.map", {"start", "map loaded"}},
        {"validatemap", {"map loaded"}},
        {"addplayer Alice Smith", {"map validated", "players added"}},
        {"gamestart", {"players added"}},
        {"replay", {"win"}},
        {"quit", {"win"}}
    };
    std::istringstream unused;
    CommandProcessor processor(unused);
    for (const auto& scenario : scenarios) {
        for (const auto& state : states) {
            bool expected = false;
            for (const auto& allowed : scenario.allowedStates) {
                if (state == allowed) expected = true;
            }
            Command command(scenario.text);
            command.saveEffect("Previous execution outcome");
            check(processor.validate(&command, state) == expected,
                  scenario.text + " in " + state);
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
    }
}

void testInvalidCommandSyntax() {
    std::istringstream unused;
    CommandProcessor processor(unused);
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
    for (const auto& scenario : malformed) {
        for (const auto& state : {scenario.state, std::string("unknown")}) {
            Command command(scenario.text);
            command.saveEffect("Old effect");
            check(!processor.validate(&command, state), "reject malformed input: " + scenario.text);
            check(command.getEffect().find(scenario.error) != std::string::npos,
                  "syntax checked before state, with a descriptive replacement effect: " + scenario.text);
        }
    }
}

void testCommandWhitespaceAndLiterals() {
    std::istringstream unused;
    CommandProcessor processor(unused);
    Command spaced(" \tloadmap \"my map\" #literal \r");
    check(processor.validate(&spaced, "start") && spaced.getEffect().empty() &&
          spaced.getArgument() == "\"my map\" #literal", "whitespace and literal arguments accepted");
    Command noArgument(" \tquit \r");
    check(processor.validate(&noArgument, "win"), "trailing whitespace is not an argument");
}

void testReadingAfterCommandRejection() {
    std::istringstream input("\nloadmap\nquit\nloadmap maps/my map.map\n");
    CommandProcessor captured(input);
    const std::string state = "start";
    int accepted = 0;
    while (Command* command = captured.getCommand()) {
        if (captured.validate(command, state)) ++accepted;
    }
    check(accepted == 1 && captured.getCommandCount() == 4 && state == "start",
          "rejections preserve state and reading continues to valid input and EOF");
    for (std::size_t i = 0; i < 3; ++i) {
        check(!captured.getSavedCommand(i)->getEffect().empty(), "history retains rejection effects");
    }
    check(captured.getSavedCommand(3)->getEffect().empty(), "accepted capture has no execution effect");
}

void reachWin(GameEngine& engine) {
    for (const std::string event : {"loadmap", "validatemap", "addplayer", "gamestart",
                                    "issueorder", "endissueorders", "win"}) {
        check(engine.processCommand(event), "win fixture transition: " + event);
    }
}

void testStoredCommandTransitions() {
    struct TransitionScenario {
        std::string text;
        std::string expectedState;
    };
    const TransitionScenario scenarios[] = {
        {" \tloadmap maps/my map.map ", "map loaded"},
        {"loadmap other.map", "map loaded"},
        {"validatemap", "map validated"},
        {"addplayer Alice Smith", "players added"},
        {"addplayer Bob", "players added"},
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
    runCommandTransitions(engine, processor, output);
    check(engine.getCurrentStateName() == "assign reinforcement" && !engine.isFinished() &&
          processor.getCommandCount() == 6 &&
          output.str().find("End of input. Command processing stopped.") != std::string::npos,
          "EOF after gamestart preserves state and reports stopped input");
    Command unavailable("loadmap missing.map");
    check(!engine.processCommand(unavailable) &&
          unavailable.getEffect().find("assign reinforcement") != std::string::npos &&
          engine.getCurrentStateName() == "assign reinforcement", "engine failure records unchanged state");
}

void testTransitionDriverRejectionAndEof() {
    std::istringstream input("\nloadmap\nquit\nloadmap my map.map\nwin\nvalidatemap");
    CommandProcessor processor(input);
    GameEngine engine;
    std::ostringstream output;
    runCommandTransitions(engine, processor, output);
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
    runCommandTransitions(engine, processor, output);
    check(processor.getCommandCount() == 6 && processor.getSavedCommand(5)->getEffect() == effect,
          "repeated EOF preserves history and effects");
    std::istringstream empty;
    CommandProcessor emptyProcessor(empty);
    GameEngine fresh;
    runCommandTransitions(fresh, emptyProcessor, output);
    check(fresh.getCurrentStateName() == "start" && emptyProcessor.getCommandCount() == 0,
          "immediate EOF preserves initial state");
}

void testTransitionDriverReplay() {
    GameEngine engine;
    reachWin(engine);
    std::istringstream replayInput("replay\nloadmap replay map.map\n");
    CommandProcessor replayProcessor(replayInput);
    std::ostringstream output;
    runCommandTransitions(engine, replayProcessor, output);
    check(engine.getCurrentStateName() == "map loaded" &&
          replayProcessor.getSavedCommand(0)->getEffect().find("'win' -> 'start'") != std::string::npos,
          "replay returns to start and continues reading");
}

void testTransitionDriverQuit() {
    GameEngine quitting;
    reachWin(quitting);
    std::istringstream quitInput("quit\nloadmap unread.map\n");
    CommandProcessor quitProcessor(quitInput);
    std::ostringstream output;
    runCommandTransitions(quitting, quitProcessor, output);
    check(quitting.isFinished() && quitProcessor.getCommandCount() == 1 &&
          quitProcessor.getSavedCommand(0)->getEffect().find("'win' -> 'end'") != std::string::npos &&
          output.str().find("Quit accepted") != std::string::npos &&
          output.str().find("End of input") == std::string::npos, "quit ends run before another read");
    check(quitProcessor.getCommand()->getText() == "loadmap unread.map", "quit leaves trailing input unread");
}

void testCommandTransitions() {
    testStoredCommandTransitions();
    testTransitionDriverRejectionAndEof();
    testTransitionDriverReplay();
    testTransitionDriverQuit();
    std::cout << "PASS: Stored-command transitions, rejection, replay, quit, and EOF.\n";
}

void testCommandValidation() {
    testCommandStateRules();
    testInvalidCommandSyntax();
    testCommandWhitespaceAndLiterals();
    testReadingAfterCommandRejection();
    std::cout << "PASS: Command syntax, state validation, and rejection checks.\n";
}

struct CommandFileFixture {
    std::filesystem::path directory;
    std::filesystem::path file;
    explicit CommandFileFixture(const std::string& contents);
    ~CommandFileFixture();
};

CommandFileFixture::CommandFileFixture(const std::string& contents) {
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

CommandFileFixture::~CommandFileFixture() {
    std::error_code error;
    std::filesystem::remove_all(directory, error);
}

void testFileReaderLinesAndCopies() {
    CommandFileFixture fixture("first\n\nlast");
    FileLineReader reader(fixture.file.string());
    std::string line;
    check(reader.readLineFromFile(line) && line == "first", "file starts at first line");
    FileLineReader copied(reader);
    FileLineReader assigned(fixture.file.string());
    assigned = reader;
    const FileLineReader& same = assigned;
    assigned = same;
    check(reader.readLineFromFile(line) && line.empty(), "blank file line retained");
    check(reader.readLineFromFile(line) && line == "last", "last line without newline retained");
    FileLineReader finalLineCopy(reader);
    check(!finalLineCopy.readLineFromFile(line), "copy after unterminated last line preserves EOF");
    check(!reader.readLineFromFile(line) && !reader.readLineFromFile(line), "repeated file EOF");
    FileLineReader exhausted(reader);
    check(!exhausted.readLineFromFile(line), "exhausted copy stays exhausted");
    for (FileLineReader* independent : {&copied, &assigned}) {
        check(independent->readLineFromFile(line) && line.empty(), "copy has independent same-position cursor");
        check(independent->readLineFromFile(line) && line == "last", "copy retains remaining order");
    }
    std::ostringstream output;
    output << reader;
    check(output.str().find("commands.txt") != std::string::npos, "reader stream output identifies file");
    CommandFileFixture empty("");
    FileLineReader emptyReader(empty.file.string());
    check(!emptyReader.readLineFromFile(line), "empty file immediately exhausts");
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

void testFileAdapterMatchesConsole() {
    const std::string text = "\nloadmap\nloadmap my map.map\nquit\nvalidatemap\naddplayer Alice Smith\ngamestart";
    CommandFileFixture fixture(text);
    FileCommandProcessorAdapter file(fixture.file.string());
    std::istringstream input(text);
    CommandProcessor console(input);
    GameEngine fileEngine;
    GameEngine consoleEngine;
    std::ostringstream fileOutput;
    std::ostringstream consoleOutput;
    runCommandTransitions(fileEngine, file, fileOutput);
    runCommandTransitions(consoleEngine, console, consoleOutput);
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

void testFileAdapterCopies() {
    CommandFileFixture fixture("\nloadmap\nloadmap my map.map\n");
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
    check(source.getSavedCommand(0)->getEffect() == "saved effect" &&
          assigned.getSavedCommand(0)->getEffect() == "saved effect" &&
          copied.getSavedCommand(0) != source.getSavedCommand(0), "adapter histories and effects are independent");
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

void testFileAdapterReplay() {
    CommandFileFixture fixture("replay\nloadmap next.map\n");
    FileCommandProcessorAdapter processor(fixture.file.string());
    GameEngine engine;
    reachWin(engine);
    std::ostringstream output;
    runCommandTransitions(engine, processor, output);
    check(engine.getCurrentStateName() == "map loaded" && processor.getCommandCount() == 2,
          "file replay continues from start");
}

void testFileAdapterQuit() {
    CommandFileFixture fixture("quit\nloadmap next.map\n");
    FileCommandProcessorAdapter processor(fixture.file.string());
    GameEngine engine;
    reachWin(engine);
    std::ostringstream output;
    runCommandTransitions(engine, processor, output);
    check(engine.isFinished() && processor.getCommandCount() == 1, "file quit stops reading");
    check(processor.getCommand()->getText() == "loadmap next.map", "file quit leaves trailing input unread");
}

void testFileFailures() {
    CommandFileFixture fixture("first\nsecond\n");
    bool threw = false;
    try { FileCommandProcessorAdapter missing((fixture.directory / "missing.txt").string()); }
    catch (const std::runtime_error& error) {
        threw = std::string(error.what()).find("missing.txt") != std::string::npos;
    }
    check(threw, "missing file reports its path");
    FileLineReader source(fixture.file.string());
    FileLineReader destination(fixture.file.string());
    std::string line;
    check(destination.readLineFromFile(line), "advance destination before failed assignment");
    FileCommandProcessorAdapter adapterSource(fixture.file.string());
    FileCommandProcessorAdapter adapterDestination(fixture.file.string());
    adapterDestination.getCommand()->saveEffect("preserved");
    std::filesystem::rename(fixture.file, fixture.directory / "moved.txt");
    threw = false;
    try { FileLineReader copy(source); } catch (const std::runtime_error&) { threw = true; }
    check(threw, "reader copy reports reopen failure");
    threw = false;
    try { destination = source; } catch (const std::runtime_error&) { threw = true; }
    check(threw && destination.readLineFromFile(line) && line == "second", "failed reader assignment preserves cursor");
    threw = false;
    try { adapterDestination = adapterSource; } catch (const std::runtime_error&) { threw = true; }
    check(threw && adapterDestination.getCommandCount() == 1 &&
          adapterDestination.getSavedCommand(0)->getEffect() == "preserved" &&
          adapterDestination.getCommand()->getText() == "second", "failed adapter assignment preserves history and cursor");
    threw = false;
    try {
        FileCommandProcessorAdapter directory(fixture.directory.string());
        directory.getCommand();
    } catch (const std::runtime_error&) { threw = true; }
    check(threw, "directory input reports an open or read error instead of EOF");
}

void testFileCommands() {
    testFileReaderLinesAndCopies();
    testFileAdapterMatchesConsole();
    testFileAdapterCopies();
    testFileAdapterReplay();
    testFileAdapterQuit();
    testFileFailures();
    std::cout << "PASS: File input, adapter transitions, independent copies, EOF, and failures.\n";
}

void demonstrateCommands(CommandProcessor& processor) {
    GameEngine engine;
    runCommandTransitions(engine, processor, std::cout);
    std::cout << processor;
}
}

bool testCommandProcessing() {
    testFileCommands();
    testCommandValidation();
    testCommandTransitions();
    std::istringstream input(" \tloadmap  maps/my map.map \t\n\n \t\naddplayer Alice Smith");
    CommandProcessor processor(input);
    Command* first = processor.getCommand();
    check(first && first->getText() == " \tloadmap  maps/my map.map \t", "original text preserved");
    check(first->getName() == "loadmap" && first->getArgument() == "maps/my map.map", "parsed views trim edges");
    check(first->getEffect().empty(), "capture does not claim execution");
    first->saveEffect("Map unavailable");
    check(processor.getSavedCommand(0)->getEffect() == "Map unavailable", "effect retained in history");
    Command valueCopy(*first);
    Command assigned("quit");
    assigned = *first;
    first->saveEffect("Changed");
    check(valueCopy.getEffect() == "Map unavailable" && assigned.getEffect() == "Map unavailable", "command copies are independent");
    const Command& same = assigned;
    assigned = same;
    check(assigned.getText() == valueCopy.getText(), "command self-assignment preserves text");
    check(processor.getCommand()->getText().empty(), "blank line is a command");
    Command* whitespace = processor.getCommand();
    check(whitespace->getText() == " \t" && whitespace->getName().empty(), "whitespace preserved");
    Command* last = processor.getCommand();
    check(last && last->getArgument() == "Alice Smith", "last unterminated line captured");
    check(!processor.getCommand() && !processor.getCommand() && processor.getCommandCount() == 4, "EOF creates no commands");
    check(first == processor.getSavedCommand(0), "borrowed pointer survives history growth");

    CommandProcessor copied(processor);
    std::istringstream otherInput("old\n");
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

    for (auto state : {std::ios::failbit, std::ios::badbit, std::ios::badbit | std::ios::eofbit}) {
        std::istringstream broken;
        broken.setstate(state);
        CommandProcessor failed(broken);
        bool threw = false;
        try { failed.getCommand(); } catch (const std::runtime_error&) { threw = true; }
        check(threw && failed.getCommandCount() == 0, "input failure throws without synthetic command");
    }
    std::istringstream masked("quit");
    masked.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);
    CommandProcessor maskedProcessor(masked);
    check(maskedProcessor.getCommand()->getText() == "quit", "exception-enabled final line captured");
    check(!maskedProcessor.getCommand(), "exception-enabled EOF is normal");
    std::istringstream shared("first\nsecond\n");
    CommandProcessor source(shared);
    CommandProcessor sharedCopy(source);
    check(source.getCommand()->getText() == "first" && sharedCopy.getCommand()->getText() == "second",
          "processor copies borrow the same input cursor");
    check(source.getCommandCount() == 1 && sharedCopy.getCommandCount() == 1, "shared input has independent histories");
    Command literal("  LOADMAP \"my map\" #literal  ");
    check(literal.getName() == "LOADMAP" && literal.getArgument() == "\"my map\" #literal", "case and literal characters preserved");
    std::cout << "PASS: Command capture, effects, copies, and input boundary checks.\n";
    return true;
}

void demonstrateConsoleCommands() {
    CommandProcessor processor;
    demonstrateCommands(processor);
}

void demonstrateFileCommands(const std::string& filename) {
    FileCommandProcessorAdapter processor(filename);
    demonstrateCommands(processor);
}

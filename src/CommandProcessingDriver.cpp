#include "CommandProcessingDriver.h"
#include "CommandProcessing.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
void check(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("Command processing check failed: " + message);
    }
}

void testCommandValidation() {
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
    Command spaced(" \tloadmap \"my map\" #literal \r");
    check(processor.validate(&spaced, "start") && spaced.getEffect().empty() &&
          spaced.getArgument() == "\"my map\" #literal", "whitespace and literal arguments accepted");
    Command noArgument(" \tquit \r");
    check(processor.validate(&noArgument, "win"), "trailing whitespace is not an argument");

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
    std::cout << "PASS: Command syntax, state validation, and rejection checks.\n";
}
}

bool testCommandProcessing() {
    testCommandValidation();
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
    std::cout << "Enter command lines; end input to stop. Capture demonstration only.\n";
    while (Command* command = processor.getCommand()) {
        command->saveEffect("Captured; not validated or executed.");
        std::cout << *command << '\n';
    }
    std::cout << "End of input. Command processing stopped.\n" << processor;
}

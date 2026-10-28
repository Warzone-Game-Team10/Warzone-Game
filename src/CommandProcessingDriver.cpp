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
}

bool testCommandProcessing() {
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

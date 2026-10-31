#include "CommandProcessing.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <utility>

namespace {
const std::string whitespace = " \t\r\n\f\v";
}

Command::Command(const std::string& text) : text(text) {} // parameterized constructor
Command::Command(const Command& other) = default; // copy constructor
Command& Command::operator=(const Command& other) = default; // assignment operator
Command::~Command() = default; // destructor

// returns the original command text
const std::string& Command::getText() const { return text; }

// returns the first word of the command without surrounding whitespace
std::string Command::getName() const {
    const auto start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    const auto end = text.find_first_of(whitespace, start);
    return text.substr(start, end == std::string::npos ? end : end - start);
}

// returns the argument after the command name without surrounding whitespace
std::string Command::getArgument() const {
    const auto start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    const auto nameEnd = text.find_first_of(whitespace, start);
    if (nameEnd == std::string::npos) return "";
    const auto argumentStart = text.find_first_not_of(whitespace, nameEnd);
    if (argumentStart == std::string::npos) return "";
    return text.substr(argumentStart, text.find_last_not_of(whitespace) - argumentStart + 1);
}

// returns the recorded command effect
const std::string& Command::getEffect() const { return effect; }

// stores the execution outcome or rejection message
void Command::saveEffect(const std::string& newEffect) { 
    effect = newEffect; 
    notify(this);// notify observer
}

// stream insertion operator
std::ostream& operator<<(std::ostream& out, const Command& command) {
    return out << "Command: " << command.text << "\n\nEffect: " << command.effect;
}



// default constructor
CommandProcessor::CommandProcessor() : CommandProcessor(std::cin) {}

// parameterized constructor
CommandProcessor::CommandProcessor(std::istream& input) : input(&input) {}

// copy constructor
CommandProcessor::CommandProcessor(const CommandProcessor& other) : input(other.input), Subject(other) {
    history.reserve(other.history.size());
    for (const auto& command : other.history) {
        history.push_back(std::make_unique<Command>(*command));
    }
}

// assignment operator
CommandProcessor& CommandProcessor::operator=(const CommandProcessor& other) {
    if (this != &other) {
        Subject::operator=(other); 
        CommandProcessor copy(other);
        history.swap(copy.history);
        input = other.input;
    }
    return *this;
}

// destructor
CommandProcessor::~CommandProcessor() = default;

/**
 * CommandProcessor reads the next input line, treating eof as the end of input.
 * Throws if reading fails for another reason
 */
bool CommandProcessor::readCommand(std::string& line) {
    try {
        std::getline(*input, line);
    } catch (const std::ios_base::failure&) {
        if (input->bad() || !input->eof()) {
            throw std::runtime_error("Failed to read command input.");
        }
    }
    if (input->bad() || (input->fail() && !input->eof())) {
        throw std::runtime_error("Failed to read command input.");
    }
    return !input->fail();
}

// saves a command in history and returns a pointer owned by the processor
Command* CommandProcessor::saveCommand(const std::string& line) {
    history.push_back(std::make_unique<Command>(line));
    notify(this);// notify observer
    return history.back().get();
}

// reads and saves the next command, or returns nullptr at the end of input
Command* CommandProcessor::getCommand() {
    std::string line;
    return readCommand(line) ? saveCommand(line) : nullptr;
}

/**
 * Checks the command syntax and allowed states.
 * Stores a rejection effect if invalid; accepted commands keep their current effect
 */
bool CommandProcessor::validate(Command* command, const std::string& currentState) {
    /**
     * Stores a command name, its argument requirement, and its allowed states
     */
    struct Rule {
        std::string name;
        bool requiresArgument;
        std::vector<std::string> states;
    };
    static const Rule rules[] = {
        {"loadmap", true, {"start", "map loaded"}},
        {"validatemap", false, {"map loaded"}},
        {"addplayer", true, {"map validated", "players added"}},
        {"gamestart", false, {"players added"}},
        {"replay", false, {"win"}},
        {"quit", false, {"win"}}
    };
    const std::string name = command->getName();
    if (name.empty()) {
        command->saveEffect("Invalid command: input is empty.");
        return false;
    }
    for (const auto& rule : rules) {
        if (name != rule.name) continue;
        const bool hasArgument = !command->getArgument().empty();
        if (hasArgument != rule.requiresArgument) {
            command->saveEffect("Invalid command '" + name + "': " +
                                (rule.requiresArgument ? "an argument is required." : "no argument is allowed."));
            return false;
        }
        for (const auto& state : rule.states) {
            if (currentState == state) return true;
        }
        command->saveEffect("Command '" + name + "' is not allowed in state '" + currentState + "'.");
        return false;
    }
    command->saveEffect("Unknown command '" + name + "'.");
    return false;
}

// returns the number of saved commands
std::size_t CommandProcessor::getCommandCount() const { return history.size(); }

// returns the saved command at the given index; throws if the index is out of range
Command* CommandProcessor::getSavedCommand(std::size_t index) { return history.at(index).get(); }

// returns the saved command at the given index; throws if the index is out of range
const Command* CommandProcessor::getSavedCommand(std::size_t index) const { return history.at(index).get(); }

// stream insertion operator
std::ostream& operator<<(std::ostream& out, const CommandProcessor& processor) {
    out << "Command history (" << processor.history.size() << "):\n\n";
    for (const auto& command : processor.history) out << *command << "\n\n";
    return out;
}



// parameterized constructor
FileLineReader::FileLineReader(const std::string& filename)
    : filename(std::filesystem::absolute(filename).string()),
      input(std::make_unique<std::ifstream>()),
      nextPosition(0), exhausted(false) {
    std::error_code error;
    if (!std::filesystem::is_regular_file(this->filename, error)) {
        throw std::runtime_error("Cannot open command file '" + this->filename + "'.");
    }
    input->open(this->filename, std::ios::binary);
    if (!input->is_open()) {
        throw std::runtime_error("Cannot open command file '" + this->filename + "'.");
    }
}

// copy constructor
FileLineReader::FileLineReader(const FileLineReader& other) : FileLineReader(other.filename) {
    nextPosition = other.nextPosition;
    exhausted = other.exhausted;
    if (!exhausted) {
        input->seekg(nextPosition);
        if (!*input) {
            throw std::runtime_error("Cannot position copied command file '" + filename + "'.");
        }
    }
}

// assignment operator
FileLineReader& FileLineReader::operator=(const FileLineReader& other) {
    if (this != &other) {
        FileLineReader copy(other);
        filename.swap(copy.filename);
        input.swap(copy.input);
        std::swap(nextPosition, copy.nextPosition);
        std::swap(exhausted, copy.exhausted);
    }
    return *this;
}

// destructor
FileLineReader::~FileLineReader() = default;

/**
 * FileLineReader reads the next file line and tracks the position for reader copies.
 * Returns false at eof and throws on a read or position failure
 */
bool FileLineReader::readLineFromFile(std::string& line) {
    if (exhausted) return false;
    std::getline(*input, line);
    if (input->bad() || (input->fail() && !input->eof())) {
        throw std::runtime_error("Failed to read command file '" + filename + "'.");
    }
    exhausted = input->eof();
    if (input->fail()) return false;
    if (!exhausted) {
        nextPosition = input->tellg();
        if (nextPosition == std::streamoff(-1)) {
            throw std::runtime_error("Cannot determine position in command file '" + filename + "'.");
        }
    }
    return true;
}

// stream insertion operator
std::ostream& operator<<(std::ostream& out, const FileLineReader& reader) {
    return out << "Command file: " << reader.filename << "\nExhausted: " << reader.exhausted;
}

// parameterized constructor
FileCommandProcessorAdapter::FileCommandProcessorAdapter(const std::string& filename)
    : reader(std::make_unique<FileLineReader>(filename)) {}

// copy constructor
FileCommandProcessorAdapter::FileCommandProcessorAdapter(const FileCommandProcessorAdapter& other)
    : CommandProcessor(other), reader(std::make_unique<FileLineReader>(*other.reader)) {}

// assignment operator
FileCommandProcessorAdapter& FileCommandProcessorAdapter::operator=(const FileCommandProcessorAdapter& other) {
    if (this != &other) {
        auto replacement = std::make_unique<FileLineReader>(*other.reader);
        CommandProcessor::operator=(other);
        reader.swap(replacement);
    }
    return *this;
}

// destructor
FileCommandProcessorAdapter::~FileCommandProcessorAdapter() = default;

// reads the next command line through the file reader
bool FileCommandProcessorAdapter::readCommand(std::string& line) {
    return reader->readLineFromFile(line);
}

// stream insertion operator
std::ostream& operator<<(std::ostream& out, const FileCommandProcessorAdapter& adapter) {
    return out << *adapter.reader << '\n' << static_cast<const CommandProcessor&>(adapter);
}


// ======== Logging Observer part ========

//Text logged when a command's effect is saved. 
std::string Command::stringToLog() const {
    return "SaveEffect: " + effect;
}

//Text logged when a command is saved: only the newest command.
std::string CommandProcessor::stringToLog() const {
    if (history.empty()){
      return "Command: (none)";  
    } 
    return "SaveCommand(): " + history.back()->getText();
}
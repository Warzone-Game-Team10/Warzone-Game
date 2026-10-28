#include "CommandProcessing.h"

#include <iostream>
#include <stdexcept>

namespace {
const std::string whitespace = " \t\r\n\f\v";
}

Command::Command(const std::string& text) : text(text) {}
Command::Command(const Command& other) = default;
Command& Command::operator=(const Command& other) = default;
Command::~Command() = default;

const std::string& Command::getText() const { return text; }

std::string Command::getName() const {
    const auto start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    const auto end = text.find_first_of(whitespace, start);
    return text.substr(start, end == std::string::npos ? end : end - start);
}

std::string Command::getArgument() const {
    const auto start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    const auto nameEnd = text.find_first_of(whitespace, start);
    if (nameEnd == std::string::npos) return "";
    const auto argumentStart = text.find_first_not_of(whitespace, nameEnd);
    if (argumentStart == std::string::npos) return "";
    return text.substr(argumentStart, text.find_last_not_of(whitespace) - argumentStart + 1);
}

const std::string& Command::getEffect() const { return effect; }
void Command::saveEffect(const std::string& newEffect) { effect = newEffect; }

std::ostream& operator<<(std::ostream& out, const Command& command) {
    return out << "Command: " << command.text << "\nEffect: " << command.effect;
}

CommandProcessor::CommandProcessor() : CommandProcessor(std::cin) {}
CommandProcessor::CommandProcessor(std::istream& input) : input(&input) {}

CommandProcessor::CommandProcessor(const CommandProcessor& other) : input(other.input) {
    history.reserve(other.history.size());
    for (const auto& command : other.history) {
        history.push_back(std::make_unique<Command>(*command));
    }
}

CommandProcessor& CommandProcessor::operator=(const CommandProcessor& other) {
    if (this != &other) {
        CommandProcessor copy(other);
        history.swap(copy.history);
        input = other.input;
    }
    return *this;
}

CommandProcessor::~CommandProcessor() = default;

bool CommandProcessor::readCommand(std::string& line) {
    // Streams with exception masks still distinguish normal EOF from failure.
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

Command* CommandProcessor::saveCommand(const std::string& line) {
    history.push_back(std::make_unique<Command>(line));
    return history.back().get();
}

Command* CommandProcessor::getCommand() {
    std::string line;
    return readCommand(line) ? saveCommand(line) : nullptr;
}

std::size_t CommandProcessor::getCommandCount() const { return history.size(); }
Command* CommandProcessor::getSavedCommand(std::size_t index) { return history.at(index).get(); }
const Command* CommandProcessor::getSavedCommand(std::size_t index) const { return history.at(index).get(); }

std::ostream& operator<<(std::ostream& out, const CommandProcessor& processor) {
    out << "Command history (" << processor.history.size() << "):\n";
    for (const auto& command : processor.history) out << *command << '\n';
    return out;
}

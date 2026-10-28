#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class Command {
private:
    std::string text;
    std::string effect;

public:
    explicit Command(const std::string& text = "");
    Command(const Command& other);
    Command& operator=(const Command& other);
    ~Command();
    const std::string& getText() const;
    std::string getName() const;
    std::string getArgument() const;
    const std::string& getEffect() const;
    void saveEffect(const std::string& effect);
    friend std::ostream& operator<<(std::ostream& out, const Command& command);
};

class CommandProcessor {
private:
    std::istream* input;
    std::vector<std::unique_ptr<Command>> history;
    virtual bool readCommand(std::string& line);
    Command* saveCommand(const std::string& line);

public:
    CommandProcessor();
    explicit CommandProcessor(std::istream& input);
    CommandProcessor(const CommandProcessor& other);
    CommandProcessor& operator=(const CommandProcessor& other);
    virtual ~CommandProcessor();
    Command* getCommand();
    bool validate(Command* command, const std::string& currentState);
    std::size_t getCommandCount() const;
    Command* getSavedCommand(std::size_t index);
    const Command* getSavedCommand(std::size_t index) const;
    friend std::ostream& operator<<(std::ostream& out, const CommandProcessor& processor);
};

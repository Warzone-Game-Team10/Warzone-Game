#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

/**
 * The Command class stores a game command and its recorded effect
 */
class Command {
private:
    std::string text;
    std::string effect;

public:
    explicit Command(const std::string& text = ""); // parameterized constructor
    Command(const Command& other); // copy constructor
    Command& operator=(const Command& other); // assignment operator
    ~Command(); // destructor

    const std::string& getText() const; // returns the original command text
    std::string getName() const; // returns the command name without surrounding whitespace
    std::string getArgument() const; // returns the text after the command name without surrounding whitespace
    const std::string& getEffect() const; // returns the recorded command effect
    void saveEffect(const std::string& effect); // stores the execution outcome or rejection message

    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const Command& command);
};

/**
 * The CommandProcessor class reads, stores, and validates game commands
 */
class CommandProcessor {
private:
    std::istream* input;
    std::vector<std::unique_ptr<Command>> history;

protected:
    // reads the next input line and returns false at the end of input
    virtual bool readCommand(std::string& line);

    // adds a command to history and returns a pointer owned by the processor
    Command* saveCommand(const std::string& line);

public:
    CommandProcessor(); // default constructor
    explicit CommandProcessor(std::istream& input); // parameterized constructor
    CommandProcessor(const CommandProcessor& other); // copy constructor
    CommandProcessor& operator=(const CommandProcessor& other); // assignment operator
    virtual ~CommandProcessor(); // destructor

    // reads and saves the next command, or returns nullptr at the end of input
    Command* getCommand();

    // checks a non-null command against syntax and state rules, saving rejection effects
    bool validate(Command* command, const std::string& currentState);

    // returns the number of saved commands
    std::size_t getCommandCount() const;

    // returns a command owned by the processor; throws if the index is out of range
    Command* getSavedCommand(std::size_t index);

    // returns a read-only command; throws if the index is out of range
    const Command* getSavedCommand(std::size_t index) const;

    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const CommandProcessor& processor);
};

/**
 * The FileLineReader class reads command file lines in order
 */
class FileLineReader {
private:
    std::string filename;
    std::unique_ptr<std::ifstream> input;
    std::streamoff nextPosition;
    bool exhausted;

public:
    explicit FileLineReader(const std::string& filename); // parameterized constructor
    FileLineReader(const FileLineReader& other); // copy constructor
    FileLineReader& operator=(const FileLineReader& other); // assignment operator
    ~FileLineReader(); // destructor

    // reads the next line, returns false at eof, and throws on a read failure
    bool readLineFromFile(std::string& line);

    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const FileLineReader& reader);
};

/**
 * The FileCommandProcessorAdapter class adapts file input to the command processor
 */
class FileCommandProcessorAdapter : public CommandProcessor {
private:
    std::unique_ptr<FileLineReader> reader;

    // reads the next command line through the file reader
    bool readCommand(std::string& line) override;

public:
    explicit FileCommandProcessorAdapter(const std::string& filename); // parameterized constructor
    FileCommandProcessorAdapter(const FileCommandProcessorAdapter& other); // copy constructor
    FileCommandProcessorAdapter& operator=(const FileCommandProcessorAdapter& other); // assignment operator
    ~FileCommandProcessorAdapter() override; // destructor
    
    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const FileCommandProcessorAdapter& adapter);
};

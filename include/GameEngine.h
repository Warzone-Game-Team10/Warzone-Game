#pragma once

#include <iostream>
#include <map>
#include <string>

#include "LoggingObserver.h"

class Command ;

// GAME ENGINE: controls flow of the game with state, transition & command
class GameEngine : public ILoggable, public Subject{
private:
    std::string currentState;

    // for all transitions in the diagram
    std::map<std::string, std::map<std::string, std::string>> transitions;

    // builds the allowed startup and play transitions
    void buildTransitions();

public:
    GameEngine(); // default constructor
    GameEngine(const GameEngine& other); //copy constructor
    GameEngine& operator=(const GameEngine& other); //assignment operator
    ~GameEngine(); //destructor

    // applies a matching transition and returns false if none exists
    bool processCommand(const std::string& command);

    // applies a command transition and stores its success or failure effect
    bool processCommand(Command& command);

    // returns the current game state name
    std::string getCurrentStateName() const;

    // returns whether the engine has reached the end state
    bool isFinished() const;

    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const GameEngine& e);

    string stringToLog( )override; // Inheriting the function from ILoggable

};
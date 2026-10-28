#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>

class Command;

// GAME ENGINE: controls flow of the game with state, transition & command
class GameEngine {
private:
    std::string currentState;
    // for all transitions in the diagram
    std::map<std::string, std::map<std::string, std::string>> transitions;

    void buildTransitions();

public:
    GameEngine();
    GameEngine(const GameEngine& other); //copy constructor
    GameEngine& operator=(const GameEngine& other); //assignment operator
    ~GameEngine(); //destructor

    bool processCommand(const std::string& command);
    bool processCommand(Command& command);
    std::string getCurrentStateName() const;
    bool isFinished() const;


    friend std::ostream& operator<<(std::ostream& out, const GameEngine& e);
};
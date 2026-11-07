#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "LoggingObserver.h"

class Command ;
class CommandProcessor;
class Map;
class Player;
class Deck;

// GAME ENGINE: controls flow of the game with state, transition & command
class GameEngine : public ILoggable, public Subject{
private:
    std::string currentState;

    // for all transitions in the diagram
    std::map<std::string, std::map<std::string, std::string>> transitions;

    Map* gameMap;
    std::vector<Player*> players;
    Deck* deck;
    CommandProcessor* commandProcessor;

    // builds the allowed startup and play transitions
    void buildTransitions();

    // startup command handlers: return true on success OR save an error effect & return false
    bool handleLoadMap(Command& command);
    bool handleValidateMap(Command& command);
    bool handleAddPlayer(Command& command);
    bool handleGameStart(Command& command);

public:
    GameEngine(); // default constructor
    GameEngine(const GameEngine& other); //copy constructor
    GameEngine& operator=(const GameEngine& other); //assignment operator
    ~GameEngine(); //destructor

    // sets the processor the engine reads its commands from
    void setCommandProcessor(CommandProcessor* processor);

    // cmd based startup phase: loadmap, validatemap, addplayer, gamestart
    void startupPhase();

    // applies a matching transition and returns false if none exists
    bool processCommand(const std::string& command);

    // applies a command transition and stores its success or failure effect
    bool processCommand(Command& command);

    // returns the current game state name
    std::string getCurrentStateName() const;

    // read-only access to the game setup for drivers and observers
    const Map* getGameMap() const;
    const std::vector<Player*>& getPlayers() const;

    // returns whether the engine has reached the end state
    bool isFinished() const;

    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const GameEngine& e);

    string stringToLog() const override; // Inheriting the function from ILoggable

};
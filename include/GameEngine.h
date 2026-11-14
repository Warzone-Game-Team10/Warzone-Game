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
    // Eliminated engine-owned players stay alive until pending orders are cleared.
    std::vector<Player*> retiredPlayers;
    void removeEliminatedPlayers(std::vector<Player*>& active);
    bool detectWinner(const std::vector<Player*>& active);
    void clearGame();
    CommandProcessor* commandProcessor;

    // builds the allowed startup and play transitions
    void buildTransitions();

    // deep copies other's players and points the copied map's territory owners at them
   void copyPlayersFrom(const GameEngine& other);

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
    const Deck* getDeck() const;

    // returns whether the engine has reached the end state
    bool isFinished() const;

    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const GameEngine& e);

    string stringToLog() const override; // Inheriting the function from ILoggable



    // --------------------   ASSIGNMENT 2 PART 3:  Game play : main game loop  --------------------------------

    /**
     * mainGameLoop() method
     * Implements the main game loop that cycles through:
     * 1. Reinforcement phase
     * 2. Issue orders phase
     * 3. Execute orders phase
     * Continues until one player controls all territories; elimination removes only that player.
     * Removes players with no territories.
     * Transitions to win state when game ends.
     */
    void mainGameLoop(std::vector<Player*>& players);
    // Use the startup player order and map for normal gameplay.
    void mainGameLoop();
    void reinforcementPhase();
    void issueOrdersPhase();
    void executeOrdersPhase();
    // Initialize a deterministic map/player setup for demonstrations.
    void setupGame(const Map& map, const std::vector<std::string>& names);

    /**
     * reinforcementPhase() method
     * Implements the reinforcement phase of the main game loop.
     * Calculates and assigns reinforcement units to each player based on:
     * - Number of territories owned (floor(territories/3))
     * - Continent control bonuses
     * - Minimum of 3 units guaranteed per turn
     */
    void reinforcementPhase(std::vector<Player*>& players);


    /**
     * issueOrdersPhase() method
     * Implements the issue orders phase of the main game loop.
     * Calls Player::issueOrder() in round-robin fashion until all players
     * indicate they have no more orders to issue.
     */
    void issueOrdersPhase(std::vector<Player*>& players); 



    /**
     * executeOrdersPhase() method
     * Implements the orders execution phase of the main game loop.
     * Executes orders from each player's order list in round-robin fashion.
     * All deploy orders are executed first, then other orders.
     * Orders are validated only when executed, not when issued.
     */
    void executeOrdersPhase(std::vector<Player*>& players);

};
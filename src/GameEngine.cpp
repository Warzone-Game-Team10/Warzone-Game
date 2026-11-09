#include "GameEngine.h"
#include "CommandProcessing.h"

#include "Map.h"
#include "Player.h"
#include "Cards.h"
#include "Orders.h"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <memory>
#include <random>

namespace {
// directory that holds the map files the user can choose from
const std::string MAP_DIRECTORY = "maps";
}

// constructors -----------------------------
// creates an engine in the start state and initializes its owned deck and transitions
GameEngine::GameEngine()
    : currentState("start"), gameMap(nullptr), deck(new Deck()), commandProcessor(nullptr) {
    buildTransitions();
}

// copy constructor: copies engine state and owned game objs while sharing its cmd processor
GameEngine::GameEngine(const GameEngine& other)
    : Subject(other),currentState(other.currentState), transitions(other.transitions),
      gameMap(other.gameMap ? new Map(*other.gameMap) : nullptr),
      deck(new Deck(*other.deck)),
      commandProcessor(other.commandProcessor) {
    copyPlayersFrom(other);
}

// assignment operator
GameEngine& GameEngine::operator=(const GameEngine& other) {
    if (this != &other) {
        Subject::operator=(other);
        currentState = other.currentState;
        transitions = other.transitions;
        commandProcessor = other.commandProcessor;

        // free what we own (players first: their hands point to the deck), then deep copy
        for (size_t i = 0; i < players.size(); i++) delete players[i];
        players.clear();
        delete deck;
        delete gameMap;

        gameMap = other.gameMap ? new Map(*other.gameMap) : nullptr;
        deck = new Deck(*other.deck);
        copyPlayersFrom(other);
    }
    return *this;
}

// deep copies other's players, then points every territory of the copied map at the matching copied player
// and every copied hand at this engine's own deck, runs after gameMap and deck have been copied
void GameEngine::copyPlayersFrom(const GameEngine& other) {
    std::map<const Player*, Player*> copiedPlayers; // original player -> its copy
    for (size_t i = 0; i < other.players.size(); i++) {
        Player* copy = new Player(*other.players[i]);
        copy->getCardHand()->setDeck(deck); // the copied hand must not keep the original engine's deck
        players.push_back(copy);
        copiedPlayers[other.players[i]] = copy;
    }

    if (gameMap == nullptr) {
        return;
    }
    for (const auto& territory : gameMap->getTerritories()) {
        const auto found = copiedPlayers.find(territory->getOwner());
        if (found != copiedPlayers.end()) {
            territory->setOwner(found->second);
        }
    }
}

// destructor
GameEngine::~GameEngine() {
    for (size_t i = 0; i < players.size(); i++) delete players[i];
    delete deck;
    delete gameMap;
}

// getter
std::string GameEngine::getCurrentStateName() const {
    return currentState;
}

// setter for cmd processor used to read startup commands
void GameEngine::setCommandProcessor(CommandProcessor* processor) {
    commandProcessor = processor;
}

// returns the currently loaded map without transferring ownership
const Map* GameEngine::getGameMap() const {
    return gameMap;
}

// returns players in the engine's current play order
const std::vector<Player*>& GameEngine::getPlayers() const {
    return players;
}
// returns the engine's own deck without transferring ownership
const Deck* GameEngine::getDeck() const {
    return deck;
}

// game engine content --------------------------
// transition table : transitions[fromState][command] = toState (diagram)
void GameEngine::buildTransitions() {
    // startup phase
    transitions["start"]["loadmap"] = "map loaded";
    transitions["map loaded"]["loadmap"] = "map loaded"; //self loop
    transitions["map loaded"]["validatemap"] = "map validated";
    transitions["map validated"]["addplayer"] = "players added";
    transitions["players added"]["addplayer"] = "players added";
    transitions["players added"]["gamestart"] = "assign reinforcement";

    // play phase
    transitions["assign reinforcement"]["issueorder"] = "issue orders";
    transitions["issue orders"]["issueorder"] = "issue orders";
    transitions["issue orders"]["endissueorders"] = "execute orders";
    transitions["execute orders"]["execorder"] = "execute orders";
    transitions["execute orders"]["endexecorders"] = "assign reinforcement";
    transitions["execute orders"]["win"] = "win";
    transitions["win"]["replay"] = "start";
    transitions["win"]["quit"] = "end";
}

// applies a matching transition and returns false if none exists
bool GameEngine::processCommand(const std::string& command) {

    //look through entries
    std::map<std::string, std::map<std::string, std::string>>::const_iterator state = transitions.find(currentState); 

    if (state != transitions.end()) {
        std::map<std::string, std::string>::const_iterator edge = state->second.find(command);
        if (edge != state->second.end()) {
            currentState = edge->second;
            return true;
        }
    }

    return false;
}



// returns whether the engine has reached the end state
bool GameEngine::isFinished() const {
    return currentState == "end";
}

// stream insertion operator
std::ostream& operator<<(std::ostream& out, const GameEngine& e) {
    out << "GameEngine current state: " << e.currentState;
    return out;
}

// applies a command transition and stores its success or failure effect
bool GameEngine::processCommand(Command& command) {
    const std::string before = currentState;
    const std::string name = command.getName();
    const std::string argument = command.getArgument();
    if (!processCommand(name)) {
        command.saveEffect("Transition failed: command '" + name +
                           "' has no transition in state '" + before + "'.");
        return false;
    }
    command.saveEffect("Transition: " + name +
                       (argument.empty() ? "" : " [" + argument + "]") +
                       ": '" + before + "' -> '" + currentState + "'.");
    return true;
}

// cmd based user interaction mechanism to start the game
void GameEngine::startupPhase() {
    if (commandProcessor == nullptr) {
        std::cout << "startupPhase: no command processor set.\n";
        return;
    }

    // play phase begins once gamestart moves the engine to "assign reinforcement"
    while (currentState != "assign reinforcement") {
        std::cout << "\n[" << currentState << "] Enter command: " << std::flush;
        Command* command = commandProcessor->getCommand();
        if (!command) {
            std::cout << "End of input. Startup phase stopped.\n";
            return;
        }

        // invalid in the current state: validate() saves the error effect, so just ask again
        if (!commandProcessor->validate(command, currentState)) {
            std::cout << '\n' << *command << '\n';
            continue;
        }

        // do the command's real work first, then transition only if it succeeded
        const std::string name = command->getName();
        bool succeeded = false;
        if (name == "loadmap")          succeeded = handleLoadMap(*command);
        else if (name == "validatemap") succeeded = handleValidateMap(*command);
        else if (name == "addplayer")   succeeded = handleAddPlayer(*command);
        else if (name == "gamestart")   succeeded = handleGameStart(*command);

        if (succeeded) {
            processCommand(*command);
        }
        std::cout << '\n' << *command << '\n';
    }
}


// lists available map files and loads the map named by the command argument
bool GameEngine::handleLoadMap(Command& command) {
    std::cout << "Available maps:\n";
    std::error_code error;
    if (!std::filesystem::is_directory(MAP_DIRECTORY, error) || error) {
        command.saveEffect("Error: map directory '" + MAP_DIRECTORY + "' is unavailable.");
        return false;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(MAP_DIRECTORY)) {
        if (entry.is_regular_file() && entry.path().extension() == ".map") {
            std::cout << "  "
                      << std::filesystem::relative(entry.path(), MAP_DIRECTORY).generic_string()
                      << "\n";
        }
    }

    const std::filesystem::path mapName(command.getArgument());
    if (mapName.empty() || mapName.is_absolute()) {
        command.saveEffect("Error: map file '" + command.getArgument() + "' not found.");
        return false;
    }
    for (const auto& component : mapName) {
        if (component == "..") {
            command.saveEffect("Error: map file must be inside the map directory.");
            return false;
        }
    }
    const std::filesystem::path path = std::filesystem::path(MAP_DIRECTORY) / mapName;
    if (!std::filesystem::is_regular_file(path, error) || error || path.extension() != ".map") {
        command.saveEffect("Error: map file '" + command.getArgument() + "' not found.");
        return false;
    }

    // the loader may throw on a malformed file
    std::unique_ptr<Map> loaded;
    try {
        MapLoader loader;
        loaded = loader.loadMap(path.string());
    } catch (const std::exception& e) {
        command.saveEffect("Error: could not load map file '" + command.getArgument() + "': " + e.what());
        return false;
    }
    if (!loaded) {
        command.saveEffect("Error: could not load map file '" + command.getArgument() + "'.");
        return false;
    }

    delete gameMap; // loadmap is allowed again in "map loaded": free the previous map
    gameMap = loaded.release();
    return true;
}

// validates currently loaded map
bool GameEngine::handleValidateMap(Command& command) {
    if (gameMap == nullptr || !gameMap->validate()) {
        command.saveEffect("Error: the map is invalid. Load another map.");
        return false;
    }
    return true;
}


// creates a player from the command argument with max 6 players
bool GameEngine::handleAddPlayer(Command& command) {
    const std::string playerName = command.getArgument();
    if (playerName.empty()) {
        command.saveEffect("Error: addplayer needs a player name.");
        return false;
    }
    if (players.size() >= 6) {
        command.saveEffect("Error: maximum of 6 players reached.");
        return false;
    }
    // player copies the supplied hand and order list, so keep these temporary objects on the stack
    Hand hand(deck);
    OrdersList orders;
    std::unique_ptr<Player> player(
        new Player(playerName, &hand, &orders, std::vector<Territory*>()));
    players.push_back(player.get());
    player.release();
    return true;
}

// distributes territories, randomizes play order, initializes armies, and deals cards
// gamestart transition only when this operation succeeds
bool GameEngine::handleGameStart(Command& command) {
    if (players.size() < 2) {
        command.saveEffect("Error: at least 2 players are needed to start.");
        return false;
    }
    if (gameMap == nullptr || deck->getSize() < static_cast<int>(players.size() * 2)) {
        command.saveEffect("Error: the map or enough initial cards are unavailable.");
        return false;
    }

    std::mt19937 rng((std::random_device())());

    // a)fairly distribute all the territories to the players: shuffle, then deal round-robin (counts differ by at most 1)
    std::vector<Territory*> territories;
    for (const auto& territory : gameMap->getTerritories()) {
        territories.push_back(territory.get());
    }
    std::shuffle(territories.begin(), territories.end(), rng);
    for (size_t i = 0; i < territories.size(); i++) {
        Player* owner = players[i % players.size()];
        owner->addTerritory(territories[i]);
        territories[i]->setOwner(owner);
    }

    // b) determine randomly the order of play of the players in the game
    std::shuffle(players.begin(), players.end(), rng);

    // c) give 50 initial armies to the players, which are placed in their respective reinforcement pool
    for (size_t i = 0; i < players.size(); i++) {
        players[i]->setReinforcementPool(50);
    }

    // d) let each player draw 2 initial cards from the deck using the deck’s draw() method
    for (size_t i = 0; i < players.size(); i++) {
        players[i]->getCardHand()->addCard(deck->draw());
        players[i]->getCardHand()->addCard(deck->draw());
    }

    return true;
}

// returns the engine's current state for logging.
string GameEngine::stringToLog( ) const{
    return "Game Engine: " + currentState;
}

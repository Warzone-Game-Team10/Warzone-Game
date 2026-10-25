#include "GameEngine.h"


// constructors -----------------------------
// first state is always start
GameEngine::GameEngine() : currentState("start") {
    buildTransitions();
}

GameEngine::GameEngine(const GameEngine& other)
    : currentState(other.currentState), transitions(other.transitions) {
}

// assignment operator
GameEngine& GameEngine::operator=(const GameEngine& other) {
    if (this != &other) {
        currentState = other.currentState;
        transitions = other.transitions;
    }
    return *this;
}

GameEngine::~GameEngine() {
}

// getter
std::string GameEngine::getCurrentStateName() const {
    return currentState;
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
    transitions["players added"]["assigncountries"] = "assign reinforcement";

    // play phase
    transitions["assign reinforcement"]["issueorder"] = "issue orders";
    transitions["issue orders"]["issueorder"] = "issue orders";
    transitions["issue orders"]["endissueorders"] = "execute orders";
    transitions["execute orders"]["execorder"] = "execute orders";
    transitions["execute orders"]["endexecorders"] = "assign reinforcement";
    transitions["execute orders"]["win"] = "win";
    transitions["win"]["play"] = "start";
    transitions["win"]["end"] = "end";
}

// processe command
bool GameEngine::processCommand(const std::string& command) {

    //look through entries
    std::map<std::string, std::map<std::string, std::string>>::const_iterator state = transitions.find(currentState); 

    if (state != transitions.end()) {
        std::map<std::string, std::string>::const_iterator edge = state->second.find(command);
        if (edge != state->second.end()) {
            currentState = edge->second;
            std::cout << "Transition accepted. Current state: " << currentState << std::endl;
            return true;
        }
    }

    std::cout << "Error: command '" << command << "' is not valid in state '" << currentState << "'." << std::endl;
    return false;
}



// reaches end state
bool GameEngine::isFinished() const {
    return currentState == "end";
}

// prints current state
std::ostream& operator<<(std::ostream& out, const GameEngine& e) {
    out << "GameEngine current state: " << e.currentState;
    return out;
}
#include "GameEngine.h"
#include "CommandProcessing.h"


// constructors -----------------------------
// first state is always start
GameEngine::GameEngine() : currentState("start") {
    buildTransitions();
}

// copy constructor
GameEngine::GameEngine(const GameEngine& other)
    : Subject(other),currentState(other.currentState), transitions(other.transitions) {
}

// assignment operator
GameEngine& GameEngine::operator=(const GameEngine& other) {
    if (this != &other) {
        Subject::operator=(other);
        currentState = other.currentState;
        transitions = other.transitions;
    }
    return *this;
}

// destructor
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

//Logging Observer part
string GameEngine::stringToLog( ) const{
    return "Game Engine: " + currentState;
}

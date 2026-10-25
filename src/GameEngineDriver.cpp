#include "GameEngine.h"

// type command until game reaches the end
// only valid cmds, if invalid then error
void testGameStates() {
    GameEngine engine;
    std::string command;

    std::cout << engine << std::endl;

    while (!engine.isFinished()) {
        std::cout << "\n[" << engine.getCurrentStateName() << "] Enter command: ";

        
        if (!(std::cin >> command)) {
            break;
        }

        engine.processCommand(command);
    }

    std::cout << "Game finished." << std::endl;
}
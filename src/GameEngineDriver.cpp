#include "GameEngine.h"
#include "GameEngineDriver.h"
#include "CommandProcessing.h"

void runCommandTransitions(GameEngine& engine, CommandProcessor& processor, std::ostream& out) {
    out << engine << '\n';
    while (!engine.isFinished()) {
        out << "\n[" << engine.getCurrentStateName() << "] Enter command: " << std::flush;
        Command* command = processor.getCommand();
        if (!command) {
            out << "End of input. Command processing stopped.\n";
            return;
        }
        if (processor.validate(command, engine.getCurrentStateName())) {
            engine.processCommand(*command);
        }
        out << '\n' << *command << '\n';
    }
    out << "Quit accepted. Command processing stopped.\n";
}

void testGameStates() {
    GameEngine engine;
    CommandProcessor processor;
    runCommandTransitions(engine, processor, std::cout);
}

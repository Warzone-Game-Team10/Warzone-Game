#include "GameEngine.h"
#include "GameEngineDriver.h"
#include "CommandProcessing.h"

void runCommandTransitions(GameEngine& engine, CommandProcessor& processor, std::ostream& out) {
    out << "State-transition demonstration only; actual startup operations are not performed.\n"
        << engine << '\n';
    while (!engine.isFinished()) {
        out << "\n[" << engine.getCurrentStateName() << "] Enter command: " << std::flush;
        Command* command = processor.getCommand();
        if (!command) {
            out << "End of input. Command processing stopped; actual startup did not complete "
                   "in this transition demonstration.\n";
            return;
        }
        if (processor.validate(command, engine.getCurrentStateName())) {
            engine.processCommand(*command);
        }
        out << *command << '\n';
    }
    out << "Quit accepted. Command processing stopped.\n";
}

void testGameStates() {
    GameEngine engine;
    CommandProcessor processor;
    runCommandTransitions(engine, processor, std::cout);
}

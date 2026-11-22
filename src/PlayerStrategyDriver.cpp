#include "PlayerStrategyDriver.h"

#include "Map.h"
#include "Player.h"
#include "PlayerStrategies.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

void checkStrategyCopy(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("Player strategy copy check failed: " + message);
    }
}

}

bool testPlayerStrategy() {
    Player source;
    Player assigned;
    CheaterPlayerStrategy sourceStrategy(&source);
    sourceStrategy.finishedIssuing = true;

    std::unique_ptr<PlayerStrategy> copiedStrategy(sourceStrategy.clone());
    checkStrategyCopy(copiedStrategy.get() != &sourceStrategy &&
                      copiedStrategy->player == &source &&
                      copiedStrategy->finishedIssuing,
                      "clone creates an independent strategy with copied state");
    copiedStrategy->finishedIssuing = false;
    checkStrategyCopy(sourceStrategy.finishedIssuing,
                      "changing copied state leaves the source unchanged");
    copiedStrategy->setPlayer(&assigned);
    checkStrategyCopy(copiedStrategy->player == &assigned &&
                      sourceStrategy.player == &source,
                      "rebinding a copy leaves the source strategy unchanged");

    CheaterPlayerStrategy assignedStrategy(&assigned);
    assignedStrategy.finishedIssuing = false;
    assignedStrategy = sourceStrategy;
    checkStrategyCopy(assignedStrategy.player == &assigned &&
                      assignedStrategy.finishedIssuing,
                      "strategy assignment copies state without changing its owner");

    std::cout << "PASS: Player strategy copies preserve state and rebind their owner\n";
    return true;
}


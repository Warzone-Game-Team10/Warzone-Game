#pragma once

#include <iosfwd>

class GameEngine;
class CommandProcessor;

/**
 * The runCommandTransitions method reads commands through the processor and executes valid transitions.
 * It prints command effects and stops at eof or the end state
 */
void runCommandTransitions(GameEngine& engine, CommandProcessor& processor, std::ostream& out);

// demonstrates game state transitions using console commands
void testGameStates();

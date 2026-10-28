#pragma once

#include <iosfwd>

class GameEngine;
class CommandProcessor;

void runCommandTransitions(GameEngine& engine, CommandProcessor& processor, std::ostream& out);
void testGameStates();

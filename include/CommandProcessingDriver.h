#pragma once

#include <string>

// runs checks for command input, validation, effects, transitions, and copying
bool testCommandProcessing();

// demonstrates console commands and prints their effects and saved history
void demonstrateConsoleCommands();

// demonstrates commands from a saved file and prints their effects and history
void demonstrateFileCommands(const std::string& filename);

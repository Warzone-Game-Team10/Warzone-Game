# Warzone-Game Team 10

## Project Overview

This project implements core components of the Warzone strategy game in C++ as a part of the COMP345 course.

### Components

Currently implemented:
- Map
- Player
- Orders
- Cards
- Game Engine

## Prerequisites

- C++ compiler (g++) that supports C++20
- GNU Make installed and available in your terminal

## Build and Run

From the project's root directory (where the Makefile is located), run:

`make run`

This compiles the source files and runs the Warzone test drivers with the all argument.

The currently available make commands are:
- `make` : Compiles the source files and generates warzone.exe
- `make run` : Compiles the project if needed, then runs all tests
- `make run-engine` : Compiles the project if needed, then runs test for GameEngine only
- `make clean` : Removes compiled object files (.o) and the executable

### Clean and Rebuild

To remove generated files and rebuild the project from scratch, run:

```
make clean
make run
```

## Project Structure

- `src/` — C++ source files (.cpp)

- `include/` — Header files (.h / .hpp)

- `Makefile` — Build, run, and cleanup commands

## Team members


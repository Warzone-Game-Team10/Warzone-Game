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
- Command Processing (console and file input)

## Prerequisites

- C++ compiler (g++) that supports C++20
- GNU Make installed and available in your terminal

## Build and Run

From the project's root directory (where the Makefile is located), run:

`make run`

This compiles the source files and runs the Warzone test drivers with the all argument.

The currently available make commands are:
- `make` or `make output` : Compiles the source files and generates warzone-tests.exe
- `make run` : Compiles the project if needed, then runs all tests, including command-processing checks
- `make run-engine` : Compiles the project if needed, then runs test for GameEngine only
- `make clean` : Removes compiled object files (.o) and the executable

### Command Processing

Build with `make output`, then choose one input mode. Replace the placeholder
file path with an existing text file containing one command per line.

macOS:

```sh
./warzone-tests.exe -console
./warzone-tests.exe -file "<path-to-command-file>"
```

Windows PowerShell:

```powershell
.\warzone-tests.exe -console
.\warzone-tests.exe -file "<path-to-command-file>"
```

End console input with Ctrl+D on macOS/Linux, or Ctrl+Z then Enter on Windows.
File input stops automatically at EOF.

Saved examples (run from the project root):

macOS/Linux:

```sh
./warzone-tests.exe -file commands/startup.txt
./warzone-tests.exe -file commands/mixed.txt
```

Windows PowerShell:

```powershell
.\warzone-tests.exe -file commands\startup.txt
.\warzone-tests.exe -file commands\mixed.txt
```

To create a temporary input file and run it, paste one of these blocks.
Edit the command lines as needed.

macOS/Linux:

```sh
cat > /tmp/warzone-commands.txt <<'EOF'
loadmap demo.map
validatemap
addplayer Jane
addplayer John
gamestart
EOF
./warzone-tests.exe -file /tmp/warzone-commands.txt
```

Windows PowerShell:

```powershell
@'
loadmap demo.map
validatemap
addplayer John
addplayer Jane
gamestart
'@ | Set-Content -Encoding ascii "$env:TEMP\warzone-commands.txt"
.\warzone-tests.exe -file "$env:TEMP\warzone-commands.txt"
```

Run checks without interactive input (`.\warzone-tests.exe` on Windows):

```sh
./warzone-tests.exe commands
./warzone-tests.exe all
./warzone-tests.exe --help
```

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


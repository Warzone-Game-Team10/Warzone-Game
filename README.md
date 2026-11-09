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

## Build and Make Commands

Run these commands from the project root (the directory containing the Makefile).
GNU Make and a C++20-compatible `g++` compiler are required.

| Command | What it does |
| --- | --- |
| `make` | Builds the default target, `output`. |
| `make output` | Compiles and links the project into `warzone-tests.exe`. |
| `make run` | Builds `warzone-tests.exe` if needed, then runs its `all` test drivers. |
| `make run-engine` | Builds the executable if needed, then runs the interactive GameEngine state-transition driver. It waits for commands in the terminal. |
| `make run-startup` | Builds the executable if needed, then runs the scripted startup-phase demonstration and checks. |
| `make run-card-smoke` | Builds `card-smoke.exe` from `tests/CardIntegrationSmokeTest.cpp` and the project source files (excluding `MainDriver.cpp`), then runs the card/order integration checks. |
| `make clean` | Removes generated `.o` files, `warzone-tests.exe`, and `card-smoke.exe` using the Windows `del` command. |
| `make clear` | Removes the same generated files using `rm`; use this cleanup alias in a Unix-like shell. |

`make run-startup` runs the scripted demonstration. To enter startup commands
yourself, build with `make output` and run `.\warzone-tests.exe -startup` on
Windows or `./warzone-tests.exe -startup` on macOS/Linux. For the other
executable modes, see [Command Processing](#command-processing) or run
`.\warzone-tests.exe --help`.


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


#include "Map.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

/**
 * Test the map loader using the maps directory and its subdirectories
 */
bool testLoadMaps(const fs::path& mapDirectory) {
    std::cout << "========================================\n"
                 "   WARZONE MAP LOADER TEST\n"
                 "========================================\n";

    std::cout << "Searching for .map files in: " << fs::absolute(mapDirectory) << '\n';
    std::vector<fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(mapDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".map") {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());
    if (files.empty()) {
        std::cout << "FAIL: no .map files found\n";
        return false;
    }
    std::cout << "Found " << files.size() << " map files\n";

    MapLoader loader;
    std::size_t testNumber = 0;
    std::size_t loaded = 0;
    std::size_t rejected = 0;
    std::size_t validMaps = 0;
    std::size_t invalidGraphs = 0;
    std::size_t fileErrors = 0;

    for (const auto& file : files) {
        std::cout << "\n========================================\n"
                  << "TEST " << ++testNumber << ": " << file.generic_string()
                  << "\n========================================\n";

        try {
            auto map = loader.loadMap(file.string());
            ++loaded;
            std::cout << "Map loaded successfully\n"
                         "\n--- MAP INFORMATION ---\n"
                      << "Territories: " << map->getTerritories().size() << '\n'
                      << "Continents: " << map->getContinents().size() << '\n'
                      << "\n--- VALIDATION ---\n";
            const bool connected = map->isConnectedGraph();
            const bool continentsConnected = map->areContinentsConnected();
            const bool membership = map->eachTerritoryBelongsToOneContinent();
            const bool valid = map->validate();
            std::cout << "1. Map is mutually reachable: " << (connected ? "PASS" : "FAIL") << '\n'
                      << "2. Each continent is mutually reachable: "
                      << (continentsConnected ? "PASS" : "FAIL") << '\n'
                      << "3. Each territory belongs to one map continent: "
                      << (membership ? "PASS" : "FAIL") << '\n'
                      << "RESULT: " << (valid ? "VALID MAP" : "REJECTED - invalid graph") << '\n';
            if (valid) {
                ++validMaps;
            } else {
                ++invalidGraphs;
            }
        } catch (const std::invalid_argument& error) {
            ++rejected;
            std::cout << "RESULT: REJECTED - invalid map data\n"
                      << "Reason: " << error.what() << '\n';
        } catch (const std::runtime_error& error) {
            ++fileErrors;
            std::cout << "RESULT: FILE ERROR\n"
                      << "Reason: " << error.what() << '\n';
        }
    }

    std::cout << "\n========================================\n"
                 "TEST SUMMARY\n"
                 "========================================\n"
              << "Files tested: " << files.size() << '\n'
              << "Maps loaded: " << loaded << '\n'
              << "Valid maps: " << validMaps << '\n'
              << "Invalid graphs rejected: " << invalidGraphs << '\n'
              << "Malformed files rejected: " << rejected << '\n'
              << "File errors: " << fileErrors << '\n'
              << "\nALL FILES PROCESSED\n";
    return validMaps > 0 && fileErrors == 0;
}

// use maps/ by default, or accept the project maps directory as an argument
int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [maps-directory]\n";
        return 1;
    }
    try {
        const fs::path mapDirectory = fs::absolute(argc == 2 ? argv[1] : "maps").lexically_normal();
        return testLoadMaps(mapDirectory) ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what()
                  << "\nSupply the project maps directory, or run from the repository root.\n";
        return 1;
    }
}

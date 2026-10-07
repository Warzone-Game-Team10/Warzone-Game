#include "Map.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <tuple>
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
                      << "RESULT: " << (valid ? "VALID MAP" : "REJECTED - STRUCTURALLY INVALID MAP") << '\n';
            if (valid) {
                ++validMaps;
            } else {
                ++invalidGraphs;
            }
        } catch (const std::invalid_argument& error) {
            ++rejected;
            std::cout << "RESULT: REJECTED - COULD NOT PARSE MAP\n"
                      << "Reason: " << error.what() << '\n';
        } catch (const std::runtime_error& error) {
            ++fileErrors;
            std::cout << "RESULT: REJECTED - COULD NOT LOAD FILE\n"
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

/**
 * Verify metadata support and distinguish parsing failures from invalid graphs
 */
bool testLoaderCases(const fs::path& mapDirectory) {
    std::cout << "\n--- PARSING AND VALIDATION CHECKS ---\n";
    // filename, expected load success, expected structural validity
    const std::tuple<const char*, bool, bool> cases[] = {
        {"tests/preamble/no_preamble.map", true, true},
        {"tests/preamble/before_files.map", true, true},
        {"tests/preamble/without_files.map", true, true},
        {"tests/preamble/unknown_keyword.map", false, false},
        {"tests/invalid_maps/arbitrary_text.map", false, false},
        {"tests/invalid_maps/disconnected_map.map", true, false},
        {"tests/invalid_maps/disconnected_continent.map", true, false}
    };
    MapLoader loader;
    bool allPassed = true;
    for (const auto& [filename, expectLoad, expectValid] : cases) {
        bool passed = false;
        std::cout << filename << ": ";
        try {
            auto map = loader.loadMap((mapDirectory / filename).string());
            const bool valid = map->validate();
            passed = expectLoad && valid == expectValid;
            std::cout << "load succeeded; validate() = " << (valid ? "true" : "false");
            if (expectValid) {
                passed = passed && map->getContinents().size() == 1
                    && map->getTerritories().size() == 3;
                if (passed) {
                    for (std::size_t i = 0; i < 3; ++i) {
                        const Territory* node = map->getTerritories()[i].get();
                        passed = passed && node->getAdjacentTerritories().size() == 1
                            && node->getAdjacentTerritories()[0]
                               == map->getTerritories()[(i + 1) % 3].get();
                    }
                }
            }
        } catch (const std::invalid_argument& error) {
            passed = !expectLoad;
            std::cout << "parsing rejected input (" << error.what() << ')';
        } catch (const std::exception& error) {
            std::cout << "unexpected loading error (" << error.what() << ')';
        }
        std::cout << " - " << (passed ? "PASS" : "FAIL") << '\n';
        allPassed = allPassed && passed;
    }
    return allPassed;
}

// use maps/ by default, or accept the project maps directory as an argument
int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [maps-directory]\n";
        return 1;
    }
    try {
        const fs::path mapDirectory = fs::absolute(argc == 2 ? argv[1] : "maps").lexically_normal();
        const bool demonstrationPassed = testLoadMaps(mapDirectory);
        const bool casesPassed = testLoaderCases(mapDirectory);
        return demonstrationPassed && casesPassed ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what()
                  << "\nSupply the project maps directory, or run from the repository root.\n";
        return 1;
    }
}

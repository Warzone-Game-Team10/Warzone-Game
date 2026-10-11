#include "MapDriver.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>

namespace {

void printUsage(std::ostream& out, const char* program) {
    out << "Usage: " << program << " [map|all] [maps-directory]\n"
        << "       " << program << " --help\n"
        << "Defaults: all tests, maps/ directory\n"
        << "Available components: map\n";
}

} 

int main(int argc, char* argv[]) {
    const std::string_view command = argc > 1 ? argv[1] : "all";
    if (argc == 2 && command == "--help") {
        printUsage(std::cout, argv[0]);
        return 0;
    }
    if (argc > 3 || (command != "map" && command != "all")) {
        printUsage(std::cerr, argv[0]);
        return 1;
    }

    try {
        const std::filesystem::path mapDirectory = argc == 3 ? argv[2] : "maps";
        std::cout << "=== COMP 345 - Warzone tests ===\n"
                  << "Running: " << command << "\n\n"
                  << "--- Part 1: Map ---\n";

        const bool passed = testMaps(mapDirectory);
        std::cout << (passed ? "\nALL SELECTED TESTS PASSED\n"
                             : "\nSELECTED TESTS FAILED\n");
        return passed ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}

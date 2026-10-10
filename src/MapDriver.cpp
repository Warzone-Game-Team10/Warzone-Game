#include "Map.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace {

void check(bool condition, const std::string& description) {
    if (!condition) {
        throw std::runtime_error("requirement check failed: " + description);
    }
    std::cout << "PASS: " << description << '\n';
}

// test value copying and non-owning links
void testValueCopies() {
    Continent continent("North", 3);
    Continent copiedContinent(continent);
    Continent assignedContinent("Old", 0);
    assignedContinent = copiedContinent;
    check(assignedContinent.getName() == "North", "continent copy preserves name");
    check(assignedContinent.getControlBonus() == 3, "continent copy preserves bonus");

    Territory first(1, "A", &continent);
    Territory second(2, "B", &continent);
    first.setArmyCount(7);
    first.setOwner(nullptr);
    first.addAdjacentTerritory(&second);
    first.addAdjacentTerritory(&second);
    check(first.getAdjacentTerritories().size() == 1, "duplicate adjacency is ignored");
    Territory copied(first);
    Territory assigned(9, "Old");
    assigned = copied;
    check(assigned.getId() == 1, "territory copy preserves ID");
    check(assigned.getName() == "A", "territory copy preserves name");
    check(assigned.getArmyCount() == 7, "territory copy preserves armies");
    check(assigned.getOwner() == nullptr, "territory copy preserves unassigned owner");
    check(assigned.getContinent() == &continent, "territory copy borrows original continent");
    check(assigned.getAdjacentTerritories() == first.getAdjacentTerritories(),
          "territory copy borrows original neighbors");
}

// create a small map for copy tests
Map makeCopyExample() {
    Map map;
    Continent* continent = map.addContinent("North", 3);
    Territory* first = map.addTerritory(1, "A", continent);
    Territory* second = map.addTerritory(2, "B", continent);
    first->setArmyCount(7);
    first->addAdjacentTerritory(second);
    first->addAdjacentTerritory(first);
    second->addAdjacentTerritory(first);
    return map;
}

// test deep copying of territories, continents, and borders
void testMapCopy() {
    Map original = makeCopyExample();
    Map copy(original);
    Territory* first = original.getTerritories().at(0).get();
    Territory* copiedFirst = copy.getTerritories().at(0).get();
    check(copy.validate(), "copied graph remains valid");
    check(copiedFirst != first, "copied territory is independently owned");
    check(copiedFirst->getContinent() == copy.getContinents().at(0).get(),
          "copied territory points to copied continent");
    check(copiedFirst->getContinent() != first->getContinent(), "copied continent is independently owned");
    check(copiedFirst->getAdjacentTerritories().at(0) == copy.getTerritories().at(1).get(),
          "copied forward edge points to copied neighbor");
    check(copiedFirst->getAdjacentTerritories().at(1) == copiedFirst, "copied self-link is preserved");
    check(copy.getTerritories().at(1)->getAdjacentTerritories().at(0) == copiedFirst,
          "copied return edge completes the cycle");
    first->setArmyCount(99);
    check(copiedFirst->getArmyCount() == 7, "copied army count is independent");
}

// test assignment, self-assignment, and cleanup of the source map
void testMapAssignment() {
    Map surviving;
    surviving.addContinent("Replaced", 0);
    {
        Map source = makeCopyExample();
        surviving = source;
        check(surviving.getContinents().size() == 1, "assignment replaces old continents");
        check(surviving.getTerritories().at(0).get() != source.getTerritories().at(0).get(),
              "assignment creates independent territories");
        check(surviving.validate(), "assigned graph is valid");
        const Map& sameMap = surviving;
        surviving = sameMap;
        check(surviving.validate(), "self-assignment preserves graph validity");
    }
    check(surviving.validate(), "assigned graph survives source destruction");
    check(surviving.getTerritories().at(0)->getArmyCount() == 7,
          "assigned armies survive source destruction");
}

// test stream output for each class
void testStreams() {
    Continent continent("North", 3);
    Territory territory(1, "A", &continent);
    territory.setArmyCount(7);
    Map map;
    MapLoader loader;
    MapLoader loaderCopy(loader);
    loaderCopy = loader;
    std::ostringstream continentOutput, territoryOutput, mapOutput, loaderOutput;
    continentOutput << continent;
    territoryOutput << territory;
    mapOutput << map;
    loaderOutput << loaderCopy;
    check(continentOutput.str().find("North") != std::string::npos, "continent stream includes name");
    check(territoryOutput.str().find("armies=7") != std::string::npos, "territory stream includes armies");
    check(mapOutput.str().find("Map{") != std::string::npos, "map supports stream insertion");
    check(loaderOutput.str().find("Domination") != std::string::npos, "copied loader stream identifies format");
}

// compare each validation result with its expected value
void checkValidation(const Map& map, bool graph, bool continents, bool membership,
                     const std::string& description) {
    check(map.isConnectedGraph() == graph, description + ": whole graph");
    check(map.areContinentsConnected() == continents, description + ": continents");
    check(map.eachTerritoryBelongsToOneContinent() == membership, description + ": membership");
    check(map.validate() == (graph && continents && membership), description + ": combined validation");
}

// test valid and invalid map structures
void testValidation(const fs::path& mapDirectory) {
    std::cout << "\n--- INDIVIDUAL VALIDATION CHECKS ---\n";
    MapLoader loader;
    checkValidation(*loader.loadMap((mapDirectory / "tests/invalid_maps/disconnected_map.map").string()),
                    false, true, true, "disconnected map with connected continents");
    checkValidation(*loader.loadMap((mapDirectory / "tests/invalid_maps/disconnected_continent.map").string()),
                    true, false, true, "connected map with disconnected continent");
    Map map;
    checkValidation(map, false, false, false, "empty map is invalid");
    Continent* continent = map.addContinent("North", 0);
    Territory* first = map.addTerritory(1, "A", continent);
    checkValidation(map, true, true, true, "single territory and zero-bonus continent are valid");
    first->setContinent(nullptr);
    checkValidation(map, true, false, false, "missing continent membership is invalid");
    Continent foreign("Foreign", 1);
    first->setContinent(&foreign);
    checkValidation(map, true, false, false, "foreign continent membership is invalid");
    first->setContinent(continent);
    Territory* second = map.addTerritory(2, "B", continent);
    first->addAdjacentTerritory(second);
    checkValidation(map, false, false, true, "one-way reachability is not mutual reachability");
    second->addAdjacentTerritory(first);
    checkValidation(map, true, true, true, "mutual reachability satisfies connectivity");
    map.addContinent("Empty", 1);
    checkValidation(map, true, false, true, "empty continent is invalid");
    first->addAdjacentTerritory(nullptr);
    checkValidation(map, false, false, true, "null adjacency is rejected safely");
    Map nullCopy(map);
    check(nullCopy.getTerritories().at(0)->getAdjacentTerritories().at(1) == nullptr,
          "map copying preserves null links");
    Territory external(3, "External");
    Map foreignLinks;
    Continent* home = foreignLinks.addContinent("Home", 1);
    foreignLinks.addTerritory(1, "A", home)->addAdjacentTerritory(&external);
    checkValidation(foreignLinks, false, false, true, "foreign adjacency is rejected safely");
    Map target;
    target.addContinent("Retained", 2);
    bool rejected = false;
    try {
        target = foreignLinks;
    } catch (const std::out_of_range&) {
        rejected = true;
    }
    check(rejected && target.getContinents().size() == 1
          && target.getContinents().at(0)->getName() == "Retained",
          "failed graph assignment preserves the target");

    Map duplicates;
    Continent* left = duplicates.addContinent("Same", 1);
    Continent* right = duplicates.addContinent("Same", 1);
    duplicates.addTerritory(1, "Same", left);
    duplicates.addTerritory(1, "Same", right);
    Map duplicateCopy(duplicates);
    check(duplicateCopy.getTerritories().at(0)->getContinent() == duplicateCopy.getContinents().at(0).get()
          && duplicateCopy.getTerritories().at(1)->getContinent() == duplicateCopy.getContinents().at(1).get(),
          "graph copying distinguishes objects with duplicate labels and IDs");
}

// skip validation when parsing fails
struct ExpectedResult {
    bool parses;
    bool valid;
};
const std::map<std::string, ExpectedResult> expectations = {
    {"canada/canada.map", {true, true}},
    {"conquest/Conquest.map", {true, true}},
    {"tests/invalid_maps/arbitrary_text.map", {false, false}},
    {"tests/invalid_maps/disconnected_continent.map", {true, false}},
    {"tests/invalid_maps/disconnected_map.map", {true, false}},
    {"tests/invalid_maps/unknown_continent.map", {false, false}}
};

// print the expected result
void printExpectation(const ExpectedResult& expected) {
    std::cout << "Expected: ";
    if (!expected.parses) {
        std::cout << "parsing rejected; validation not run\n";
    } else if (expected.valid) {
        std::cout << "parsing succeeds; VALID MAP\n";
    } else {
        std::cout << "parsing succeeds; INVALID MAP\n";
    }
}

// check that C-Block -> A-Block stays one-way
bool hasConquestOneWayBorder(const Map& map) {
    const Territory* source = nullptr;
    const Territory* target = nullptr;
    for (const auto& territory : map.getTerritories()) {
        if (territory->getId() == 4) source = territory.get();
        if (territory->getId() == 2) target = territory.get();
    }
    if (!source || !target) return false;
    const auto& forward = source->getAdjacentTerritories();
    const auto& reverse = target->getAdjacentTerritories();
    return std::find(forward.begin(), forward.end(), target) != forward.end()
        && std::find(reverse.begin(), reverse.end(), source) == reverse.end();
}

} // namespace

/**
 * Map loading tests
 */
bool testLoadMaps(const fs::path& mapDirectory) {
    std::cout << "========================================\n"
                 "   WARZONE MAP LOADER TEST\n"
                 "========================================\n";

    std::cout << "Loading core fixtures from: " << fs::absolute(mapDirectory) << '\n';

    MapLoader loader;
    std::size_t testNumber = 0;
    std::size_t loaded = 0;
    std::size_t rejected = 0;
    std::size_t validMaps = 0;
    std::size_t invalidGraphs = 0;
    std::size_t fileErrors = 0;
    std::size_t passedTests = 0;
    std::size_t missingFiles = 0;

    for (const auto& [filename, expected] : expectations) {
        if (!fs::is_regular_file(mapDirectory / filename)) {
            ++missingFiles;
            std::cout << "FAIL: missing expected fixture: " << filename << '\n';
        }
    }

    for (const auto& [filename, expected] : expectations) {
        const fs::path file = mapDirectory / filename;
        std::cout << "\n========================================\n"
                  << "TEST " << ++testNumber << ": " << file.generic_string()
                  << "\n========================================\n";

        printExpectation(expected);
        bool passed = false;
        bool parsed = false;

        try {
            auto map = loader.loadMap(file.string());
            parsed = true;
            ++loaded;
            std::cout << "Map loaded successfully\n"
                         "\n--- MAP INFORMATION ---\n"
                      << "Territories: " << map->getTerritories().size() << '\n'
                      << "Continents: " << map->getContinents().size() << '\n'
                      << "\n--- VALIDATION ---\n";
            const bool connected = map->isConnectedGraph();
            const bool continentsConnected = map->areContinentsConnected();
            const bool membership = map->eachTerritoryBelongsToOneContinent();
            const bool valid = connected && continentsConnected && membership;
            passed = expected.parses
                && valid == expected.valid;
            if (filename == "conquest/Conquest.map") {
                const bool directedBorder = hasConquestOneWayBorder(*map);
                std::cout << "Conquest one-way border preserved: "
                          << (directedBorder ? "PASS" : "FAIL") << '\n';
                passed = passed && directedBorder;
            }
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
            passed = !parsed && !expected.parses;
            ++rejected;
            std::cout << "RESULT: REJECTED - COULD NOT PARSE MAP\n"
                      << "Reason: " << error.what() << '\n';
        } catch (const std::runtime_error& error) {
            ++fileErrors;
            std::cout << "RESULT: REJECTED - COULD NOT LOAD FILE\n"
                      << "Reason: " << error.what() << '\n';
        }
        std::cout << "Expectation check: " << (passed ? "PASS" : "FAIL") << '\n';
        if (passed) {
            ++passedTests;
        }
    }

    std::cout << "\n========================================\n"
                 "TEST SUMMARY\n"
                 "========================================\n"
              << "Files tested: " << expectations.size() << '\n'
              << "Maps loaded: " << loaded << '\n'
              << "Valid maps: " << validMaps << '\n'
              << "Invalid graphs rejected: " << invalidGraphs << '\n'
              << "Malformed files rejected: " << rejected << '\n'
              << "File errors: " << fileErrors << '\n'
              << "Expectation checks passed: " << passedTests << '/' << expectations.size() << '\n'
              << "Missing fixtures: " << missingFiles << '\n'
              << "\nALL FILES PROCESSED\n";
    return passedTests == expectations.size() && missingFiles == 0;
}

// use maps/ unless a directory is given
int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [maps-directory]\n";
        return 1;
    }
    try {
        const fs::path mapDirectory = fs::absolute(argc == 2 ? argv[1] : "maps").lexically_normal();
        if (!fs::is_directory(mapDirectory)) {
            std::cerr << "FAIL: maps directory does not exist or is not a directory: " << mapDirectory
                      << "\nSupply the project maps directory, or run from the repository root.\n";
            return 1;
        }
        testValueCopies();
        testMapCopy();
        testMapAssignment();
        testStreams();
        testValidation(mapDirectory);
        return testLoadMaps(mapDirectory) ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}

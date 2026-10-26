#include "Map.h"
#include <utility>

#include <algorithm>
#include <charconv>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ostream>
#include <unordered_map>
#include <unordered_set>

/**
 * Continent class implementation
 */
Continent::Continent(const std::string& name, int controlBonus)
    : name(name), controlBonus(controlBonus) {}

Continent::Continent(const Continent& other) = default;

// assignment operator: copy another continent's name and control bonus
Continent& Continent::operator=(const Continent& other) = default;

Continent::~Continent() = default;

// getters
const std::string& Continent::getName() const {
    return name;
}

int Continent::getControlBonus() const {
    return controlBonus;
}

/**
 * stream insertion operator
 */
std::ostream& operator<<(std::ostream& out, const Continent& continent) {
    return out << "Continent{name=" << continent.name
               << ", controlBonus=" << continent.controlBonus << "}";
}

/**
 * Territory class implementation
 */
Territory::Territory(int id, const std::string& name, Continent* continent)
    : id(id), name(name), continent(continent), owner(nullptr), armyCount(0),
      adjacentTerritories() {}

Territory::Territory(const Territory& other) = default;

// assignment operator: copy another territory's values and non-owning links
Territory& Territory::operator=(const Territory& other) = default;

Territory::~Territory() = default;

// getters
int Territory::getId() const {
    return id;
}

const std::string& Territory::getName() const {
    return name;
}

Continent* Territory::getContinent() const {
    return continent;
}

Player* Territory::getOwner() const {
    return owner;
}

int Territory::getArmyCount() const {
    return armyCount;
}

const std::vector<Territory*>& Territory::getAdjacentTerritories() const {
    return adjacentTerritories;
}

// setters
void Territory::setContinent(Continent* continent) {
    this->continent = continent;
}

void Territory::setOwner(Player* owner) {
    this->owner = owner;
}

void Territory::setArmyCount(int armyCount) {
    this->armyCount = armyCount;
}

// adjacency management
void Territory::setAdjacentTerritories(std::vector<Territory*> territories) {
    adjacentTerritories = std::move(territories);
}

void Territory::addAdjacentTerritory(Territory* territory) {
    if (std::find(adjacentTerritories.begin(), adjacentTerritories.end(), territory)
        == adjacentTerritories.end()) {
        adjacentTerritories.push_back(territory);
    }
}

/**
 * stream insertion operator
 */
std::ostream& operator<<(std::ostream& out, const Territory& territory) {
    out << "Territory{id=" << territory.id << ", name=" << territory.name
        << ", continent=" << (territory.continent ? "assigned" : "unassigned")
        << ", owner=" << (territory.owner ? "assigned" : "unassigned")
        << ", armies=" << territory.armyCount << ", adjacent=[";

    for (std::size_t i = 0; i < territory.adjacentTerritories.size(); ++i) {
        if (i != 0) {
            out << ", ";
        }
        const Territory* adjacent = territory.adjacentTerritories[i];
        if (adjacent) {
            out << adjacent->getId();
        } else {
            out << "null";
        }
    }

    return out << "]}";
}

/**
 * Map class implementation
 */
Map::Map() = default;

Map::Map(const Map& other) {
    std::unordered_map<const Continent*, Continent*> continentCopies;
    std::unordered_map<const Territory*, Territory*> territoryCopies;

    // copy continents and record their replacements
    for (const auto& original : other.continents) {
        Continent* copy = addContinent(original->getName(), original->getControlBonus());
        continentCopies.emplace(original.get(), copy);
    }

    // copy territories before rebuilding their links
    for (const auto& original : other.territories) {
        Territory* copy = addTerritory(original->getId(), original->getName());
        copy->setArmyCount(original->getArmyCount());
        copy->setOwner(original->getOwner());
        territoryCopies.emplace(original.get(), copy);
    }

    // rebuild continent membership and adjacency
    for (const auto& original : other.territories) {
        Territory* copy = territoryCopies.at(original.get());
        if (original->getContinent()) {
            copy->setContinent(continentCopies.at(original->getContinent()));
        }
        for (const Territory* adjacent : original->getAdjacentTerritories()) {
            Territory* copiedNeighbor = nullptr;
            if (adjacent) {
                copiedNeighbor = territoryCopies.at(adjacent);
            }
            copy->addAdjacentTerritory(copiedNeighbor);
        }
    }
}

// assignment operator: replace this map with a deep copy of another map
Map& Map::operator=(const Map& other) {
    if (this != &other) {
        Map replacement(other);
        continents.swap(replacement.continents);
        territories.swap(replacement.territories);
        // replacement automatically deletes the old objects when it leaves scope
    }
    return *this;
}

Map::~Map() = default;

// getters
const std::vector<std::unique_ptr<Continent>>& Map::getContinents() const {
    return continents;
}

const std::vector<std::unique_ptr<Territory>>& Map::getTerritories() const {
    return territories;
}

// territory management
Territory* Map::addTerritory(int id, const std::string& name, Continent* continent) {
    territories.push_back(std::make_unique<Territory>(id, name, continent));
    return territories.back().get();
}

// continent management
Continent* Map::addContinent(const std::string& name, int controlBonus) {
    continents.push_back(std::make_unique<Continent>(name, controlBonus));
    return continents.back().get();
}

/**
 * stream insertion operator
 */
std::ostream& operator<<(std::ostream& out, const Map& map) {
    return out << "Map{continents=" << map.continents.size()
               << ", territories=" << map.territories.size() << "}";
}

// checks reachability (both forward and backward) to establish mutual reachability between territories
bool Map::isConnected(const std::vector<const Territory*>& nodes) {
    if (nodes.empty()) {
        return false;
    }
    const std::unordered_set<const Territory*> members(nodes.begin(), nodes.end());
    std::unordered_map<const Territory*, std::vector<const Territory*>> reverseEdges;
    for (const Territory* node : nodes) {
        for (const Territory* neighbor : node->getAdjacentTerritories()) {
            if (members.contains(neighbor)) {
                reverseEdges[neighbor].push_back(node);
            }
        }
    }

    const auto reachesAll = [&](bool reverse) {
        std::unordered_set<const Territory*> visited;
        std::vector<const Territory*> pending{nodes.front()};
        visited.insert(nodes.front());
        while (!pending.empty()) {
            const Territory* current = pending.back();
            pending.pop_back();
            if (reverse) {
                for (const Territory* neighbor : reverseEdges[current]) {
                    if (visited.insert(neighbor).second) {
                        pending.push_back(neighbor);
                    }
                }
            } else {
                for (const Territory* neighbor : current->getAdjacentTerritories()) {
                    if (members.contains(neighbor) && visited.insert(neighbor).second) {
                        pending.push_back(neighbor);
                    }
                }
            }
        }
        return visited.size() == nodes.size();
    };
    return reachesAll(false) && reachesAll(true);
}

// validates the connectivity of the entire directed territory graph
bool Map::isConnectedGraph() const {
    if (!hasValidLinks()) {
        return false;
    }
    return hasConnectedTerritories();
}

// traverse owned territories after checking links
bool Map::hasConnectedTerritories() const {
    std::vector<const Territory*> nodes;
    for (const auto& territory : territories) {
        nodes.push_back(territory.get());
    }
    return isConnected(nodes);
}

// checks that all territories within each continent are connected
bool Map::areContinentsConnected() const {
    if (!eachTerritoryBelongsToOneContinent() || !hasValidLinks()) {
        return false;
    }
    return hasConnectedContinents();
}

// traverse each continent after checking membership and links
bool Map::hasConnectedContinents() const {
    for (const auto& continent : continents) {
        std::vector<const Territory*> nodes;
        for (const auto& territory : territories) {
            if (territory->getContinent() == continent.get()) {
                nodes.push_back(territory.get());
            }
        }
        if (!isConnected(nodes)) {
            return false;
        }
    }
    return true;
}

// validate: combines membership and connectivity requirements
bool Map::validate() const {
    return eachTerritoryBelongsToOneContinent()
        && hasValidLinks() && hasConnectedTerritories() && hasConnectedContinents();
}

// validates that every territory is assigned to exactly one continent in this map
bool Map::eachTerritoryBelongsToOneContinent() const {
    if (territories.empty() || continents.empty()) {
        return false;
    }
    std::unordered_set<const Continent*> ownedContinents;
    for (const auto& continent : continents) {
        ownedContinents.insert(continent.get());
    }
    for (const auto& territory : territories) {
        if (!ownedContinents.contains(territory->getContinent())) {
            return false;
        }
    }
    return true;
}

// checks that every adjacent territory belongs to this map
bool Map::hasValidLinks() const {
    std::unordered_set<const Territory*> ownedTerritories;
    for (const auto& territory : territories) {
        ownedTerritories.insert(territory.get());
    }
    for (const auto& territory : territories) {
        for (const Territory* neighbor : territory->getAdjacentTerritories()) {
            if (!ownedTerritories.contains(neighbor)) {
                return false;
            }
        }
    }
    return true;
}

/**
 * MapLoader class implementation
 */
MapLoader::MapLoader() = default;

MapLoader::MapLoader(const MapLoader& other) = default;

// assignment operator
MapLoader& MapLoader::operator=(const MapLoader& other) = default;

MapLoader::~MapLoader() = default;

// map loading
std::unique_ptr<Map> MapLoader::loadMap(const std::string& filename) const {
    std::ifstream input(filename);
    if (!input) {
        throw std::runtime_error(filename + ": cannot open map file");
    }

    auto map = std::make_unique<Map>();
    ParseState currentState = ParseState::None;
    std::unordered_map<int, Territory*> countries;
    std::string line;
    std::size_t lineNumber = 0;

    // parse the file line by line, if parsing fails --> the partially built map is cleaned up automatically
    try {
        while (std::getline(input, line)) {
            ++lineNumber;
            if (lineNumber == 1 && line.starts_with("\xEF\xBB\xBF")) {
                line.erase(0, 3);
            }
            const auto fields = splitLine(line);
            if (fields.empty()) {
                continue;
            }
            if (fields[0].starts_with('[')) {
                currentState = parseSection(fields, currentState);
                continue;
            }

            switch (currentState) {
                case ParseState::Files:
                    break; 
                case ParseState::Continents:
                    parseContinentSection(*map, fields);
                    break;
                case ParseState::Countries:
                    parseCountrySection(*map, fields, countries);
                    break;
                case ParseState::Borders:
                    parseBorderSection(fields, countries);
                    break;
                case ParseState::None:
                    parsePreamble(fields[0]);
                    break;
            }
        }
        if (input.bad()) {
            throw std::runtime_error(filename + ": map read failed");
        }
        if (currentState != ParseState::Borders) {
            throw std::invalid_argument("missing required map sections");
        }
    } catch (const std::invalid_argument& error) {
        throw std::invalid_argument(filename + ":" + std::to_string(lineNumber)
                                    + ": " + error.what());
    }
    return map;
}

// splits the line into fields and ignore anything after a semicolon comment
std::vector<std::string> MapLoader::splitLine(const std::string& line) {
    std::istringstream row(line.substr(0, line.find(';')));
    std::vector<std::string> fields;
    std::string field;
    while (row >> field) {
        fields.push_back(field);
    }
    return fields;
}

// ignore recognized metadata because it is not part of the graph model
void MapLoader::parsePreamble(const std::string& keyword) {
    if (keyword != "test" && keyword != "name" && keyword != "ver" && keyword != "circle") {
        throw std::invalid_argument("data before a recognized section");
    }
}

// converts the token to an integer and validate that the whole token is numeric
int MapLoader::parseInteger(const std::string& token) {
    int value = 0;
    const auto result = std::from_chars(token.data(), token.data() + token.size(), value);
    if (result.ec != std::errc() || result.ptr != token.data() + token.size()) {
        throw std::invalid_argument("invalid integer: " + token);
    }
    return value;
}

MapLoader::ParseState MapLoader::parseSection(const std::vector<std::string>& fields,
                                              ParseState currentState) {
    if (fields.size() != 1) {
        throw std::invalid_argument("invalid section header");
    }
    const std::string& header = fields[0];
    if (header == "[files]" && currentState == ParseState::None) {
        return ParseState::Files;
    }
    if (header == "[continents]"
        && (currentState == ParseState::None || currentState == ParseState::Files)) {
        return ParseState::Continents;
    }
    if (header == "[countries]" && currentState == ParseState::Continents) {
        return ParseState::Countries;
    }
    if (header == "[borders]" && currentState == ParseState::Countries) {
        return ParseState::Borders;
    }
    throw std::invalid_argument("unknown, repeated, or out-of-order section: " + header);
}

// reads the continent name, control bonus, and display color
void MapLoader::parseContinentSection(Map& map, const std::vector<std::string>& fields) {
    if (fields.size() != 3) {
        throw std::invalid_argument("expected continent name, bonus, and color");
    }
    const int bonus = parseInteger(fields[1]);
    if (bonus < 0) {
        throw std::invalid_argument("negative continent bonus");
    }
    map.addContinent(fields[0], bonus);
}

// create a territory and record its explicit ID for border lookup
void MapLoader::parseCountrySection(Map& map, const std::vector<std::string>& fields,
                                    std::unordered_map<int, Territory*>& countries) {
    if (fields.size() != 5) {
        throw std::invalid_argument("expected country ID, name, continent ID, x, and y");
    }
    const int id = parseInteger(fields[0]);
    const int continentId = parseInteger(fields[2]);
    parseInteger(fields[3]); // coordinates are read but not stored
    parseInteger(fields[4]);
    if (id <= 0 || countries.contains(id)) {
        throw std::invalid_argument("invalid or duplicate country ID: " + fields[0]);
    }
    if (continentId <= 0
        || static_cast<std::size_t>(continentId) > map.getContinents().size()) {
        throw std::invalid_argument("unknown continent ID: " + fields[2]);
    }
    // continent IDs are based on the index of the continent in the vector
    Continent* continent = map.getContinents()[continentId - 1].get();
    countries.emplace(id, map.addTerritory(id, fields[1], continent));
}

// link existing territories in the direction listed by each border record
void MapLoader::parseBorderSection(const std::vector<std::string>& fields,
                                   const std::unordered_map<int, Territory*>& countries) {
    const int sourceId = parseInteger(fields[0]);
    if (!countries.contains(sourceId)) {
        throw std::invalid_argument("unknown border source: " + fields[0]);
    }
    Territory* source = countries.at(sourceId);
    for (std::size_t i = 1; i < fields.size(); ++i) {
        const int targetId = parseInteger(fields[i]);
        if (!countries.contains(targetId)) {
            throw std::invalid_argument("unknown border target: " + fields[i]);
        }
        source->addAdjacentTerritory(countries.at(targetId));
    }
}

/**
 * stream insertion operator
 */
std::ostream& operator<<(std::ostream& out, const MapLoader&) {
    return out << "MapLoader{format=Domination}";
}

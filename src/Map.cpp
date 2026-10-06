#include "Map.h"

#include <algorithm>
#include <ostream>
#include <unordered_map>

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

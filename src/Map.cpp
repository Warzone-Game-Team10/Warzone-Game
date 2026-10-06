#include "Map.h"

#include <algorithm>
#include <ostream>

/**
 * Continent class implementation
 */
Continent::Continent(const std::string& name, int controlBonus)
    : name(name), controlBonus(controlBonus) {}

Continent::Continent(const Continent& other) = default;

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

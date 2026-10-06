#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class Player;

/**
 * The Continent class represents a connected subgraph of territories 
 */
class Continent {
private:
    std::string name;
    int controlBonus;

public:
    Continent(const std::string& name, int controlBonus);
    Continent(const Continent& other); // copy constructor
    Continent& operator=(const Continent& other); // assignment operator
    ~Continent(); // destructor

    // getters
    const std::string& getName() const;
    int getControlBonus() const;

    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const Continent& continent);
};

/**
 * The Territory class represents a node in the map graph
 */
class Territory {
private:
    int id;
    std::string name;
    Continent* continent;
    Player* owner;
    int armyCount;
    std::vector<Territory*> adjacentTerritories;

public:
    Territory(int id, const std::string& name, Continent* continent = nullptr);
    Territory(const Territory& other); // copy constructor
    Territory& operator=(const Territory& other); // assignment operator
    ~Territory(); // destructor

    // getters
    int getId() const;
    const std::string& getName() const;
    Continent* getContinent() const;
    Player* getOwner() const;
    int getArmyCount() const;
    const std::vector<Territory*>& getAdjacentTerritories() const;

    // setters
    void setContinent(Continent* continent);
    void setOwner(Player* owner);
    void setArmyCount(int armyCount);

    // adjacency management
    void addAdjacentTerritory(Territory* territory);

    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const Territory& territory);
};

/**
 * The Map class represents the game map as a connected graph of territories grouped into continents
 */
class Map {
private:
    std::vector<std::unique_ptr<Continent>> continents;
    std::vector<std::unique_ptr<Territory>> territories;

public:
    Map();
    Map(const Map& other); // copy constructor
    Map& operator=(const Map& other); // assignment operator
    ~Map(); // destructor

    // getters
    const std::vector<std::unique_ptr<Continent>>& getContinents() const;
    const std::vector<std::unique_ptr<Territory>>& getTerritories() const;

    // object creation
    Continent* addContinent(const std::string& name, int controlBonus);
    Territory* addTerritory(int id, const std::string& name,
                            Continent* continent = nullptr);

    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const Map& map);
};

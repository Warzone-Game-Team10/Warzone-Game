#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>
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
    // Replace the neighbour list so copied territories can point to each other.
    void setAdjacentTerritories(std::vector<Territory*> territories);

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

    // validation helpers
    bool hasValidLinks() const;
    // traversal helpers
    bool hasConnectedTerritories() const;
    bool hasConnectedContinents() const;
    static bool isConnected(const std::vector<const Territory*>& nodes);

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

    bool isConnectedGraph() const;
    bool areContinentsConnected() const;
    bool eachTerritoryBelongsToOneContinent() const;
    bool validate() const;

    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const Map& map);
};

/**
 * The MapLoader class reads Domination map files and builds map objects
 */
class MapLoader {
private:
    enum class ParseState { None, Files, Continents, Countries, Borders };

    // parsing helpers
    static std::vector<std::string> splitLine(const std::string& line);
    static void parsePreamble(const std::string& keyword);
    static int parseInteger(const std::string& token);
    static ParseState parseSection(const std::vector<std::string>& fields,
                                   ParseState currentState);
    static void parseContinentSection(Map& map, const std::vector<std::string>& fields);
    static void parseCountrySection(Map& map, const std::vector<std::string>& fields,
                                    std::unordered_map<int, Territory*>& countries);
    static void parseBorderSection(const std::vector<std::string>& fields,
                                   const std::unordered_map<int, Territory*>& countries);

public:
    MapLoader();
    MapLoader(const MapLoader& other); // copy constructor
    MapLoader& operator=(const MapLoader& other); // assignment operator
    ~MapLoader(); // destructor

    // caller owns the parsed map; call validate() separately for structural rules
    std::unique_ptr<Map> loadMap(const std::string& filename) const;

    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const MapLoader& loader);
};

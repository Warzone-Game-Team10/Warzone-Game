 #pragma once
#include <stdio.h>
#include <vector>
#include <string>


using namespace std;

class Territory {
private:
    string name;
 
public:
    Territory(string name) {
        this->name = name;
    }
    string getName() {
        return name;
    }
};

class Map {
    private:
        string mapName;

    public:
        //constructor
        Map() {
            mapName = "Default Map";
        }
        
};
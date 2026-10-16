#pragma once
#include <stdio.h>
#include <vector>
#include <string>


using namespace std;

class Order {
    private:
        vector<string> type;

    public:
        //constructor
       Order() {
            type.push_back("remove");
            type.push_back("execute");
            type.push_back("validate");
        }       
       

};

class OrdersList {
private:
    vector<Order*> orders;
 
public:
    OrdersList() {
        
    }
    void add(Order* order) {
        orders.push_back(order);
    }
    
};
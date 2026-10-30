#pragma once

#include <stdio.h>
#include <vector>
#include <string>

using namespace std; 

//when obser get message it updates
class Observer{
    public:
    virtual ~Observer();
    virtual void update(ILoggable* loggable) = 0;
};

//interface that must be inherited by Order, OrderList, GameEngine, Command, Command processor (subject)
class ILoggable{
    // pure virtual function providing interface framework.
    public:
    virtual ~ILoggable();
    virtual string stringToLog() const = 0;

};

//when subject changes state it notifies the observer
class Subject{
    public:
    Subject();
    Subject(const Subject& other);              // copies observer pointers
    Subject& operator=(const Subject& other);
    virtual ~Subject();                         // delete the vector only, not observers
    void attach(Observer* observer);
    void detach(Observer* observer);
    void notify(ILoggable* loggable);           // calls update() on every observer
    const vector<Observer*>& getObservers() const; // handy to pass observers to child objects
    friend ostream& operator<<(std::ostream& os, const Subject& s);
private:
    vector<Observer*>* observers;
};


// Concrete observer: appends to gamelog.txt. 
class LogObserver : public Observer {
public:
    LogObserver();
    LogObserver(const std::string& fileName);
    LogObserver(const LogObserver& other);
    LogObserver& operator=(const LogObserver& other);
    ~LogObserver();
    void update(ILoggable* loggable) override;  // open file (ios::app), write stringToLog(), close
    friend ostream& operator<<(std::ostream& os, const LogObserver& o);

};
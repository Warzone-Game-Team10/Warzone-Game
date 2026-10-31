#pragma once

#include <stdio.h>
#include <vector>
#include <string>

using namespace std; 

class ILoggable;    // forward declaration (Observer only uses a pointer to it)
class Observer;     // forward declaration

//==== OBSERVER -INTERFACE ====
//when obser get message it updates
class Observer{
    public:
    virtual ~Observer();
    //pure virtual that Log observer will need to overide
    virtual void update(ILoggable* loggable) const = 0;
};

//======ILOGGABLE - INTERFACE======
//interface that must be inherited by Order, OrderList, GameEngine, Command, Command processor (subject)
class ILoggable{
    public:
    virtual ~ILoggable();
    
    // pure virtual function providing interface framework.
    virtual string stringToLog() const = 0;

};

//=====SUBJECT======
//when subject changes state it notifies the observer
class Subject{
    public:

    //constructor
    Subject();
    //deep copy constrcutor
    Subject(const Subject& other);             
    Subject& operator=(const Subject& other);
    //distructor
    virtual ~Subject(); //A base class with children should have a virtual destructor, and it is also what makes Subject polymorphic so the driver’s casts compile                         
    void attach(Observer* observer);
    void detach(Observer* observer);
    void notify(ILoggable* loggable);// calls update() on every observer
    const vector<Observer*>& getObservers() const; // handy to pass observers to child objects
    friend ostream& operator<<(std::ostream& os, const Subject& s);
private:
    vector<Observer*>* observers;
};

//=====LOGOBSERVER======
// Concrete observer: appends to gamelog.txt. and inheriace form Observer
class LogObserver : public Observer {
public:
    //constructor
    LogObserver();
    //copy constructor
    LogObserver(const LogObserver& other);
    LogObserver& operator=(const LogObserver& other);

    //distructor
    ~LogObserver();
    void update(ILoggable* loggable) const override;  // open file (ios::app), write stringToLog(), close
    friend ostream& operator<<(std::ostream& os, const LogObserver& o);

};
#include "LoggingObserver.h"
#include <algorithm>
#include <fstream>
#include <vector>


namespace {
    //filename to writ elogs into
    const char* const LOG_FILE_NAME = "gamelog.txt";
}
using namespace std; 

// ======== ILoggable -INTERFACE ===========
ILoggable::~ILoggable() {}
//no need for dtringToLog since it interface

// =========== Observer -INTERFACE===========
Observer::~Observer() {}


// =========== Subject ===========

/** Creates a subject with an empty observer list. */
Subject::Subject() {
    observers = new vector<Observer*>();    
}

// Deep copy
Subject::Subject(const Subject& other){
    observers = new vector<Observer*>();

    //size_t is int type to represent the size of any obj in memory. SO it will be large enough for sizes of vetor
    for (size_t count =0; count < other.observers->size(); count++){
        //add to new observer vector
        observers->push_back((*other.observers)[count]);
    }

}


Subject& Subject::operator=(const Subject& other) {
    //if theirnot equal
    if (this != &other) {
        observers->clear();
 
        //deep copy
        for (size_t i = 0; i < other.observers->size(); i++) {
            observers->push_back((*other.observers)[i]);
        }
    }
    return *this;
}

//destructor
Subject::~Subject() {
    delete observers;
    observers=nullptr;
}

//Registers an observer so it gets notified of changes.
void Subject::attach(Observer* observer) {
   
    if (observer == nullptr) {
        return;
    }
    // avoid attaching the same observer twice 
    for (size_t i = 0; i < observers->size(); i++) {
        if ((*observers)[i] == observer) {
            return;
        }
    }
 
    //add observer to the list
    observers->push_back(observer);
}

// Removes an observer from the list (does nothing if it is not in the list).
void Subject::detach(Observer* observer) {
    for (size_t i = 0; i < observers->size(); i++) {
        if ((*observers)[i] == observer) {
            observers->erase(observers->begin() + i);
            return;
        }
    }
}

// Calls update() on every attached observer.
void Subject::notify(ILoggable* loggable) {
    for (size_t i = 0; i < observers->size(); i++) {
        (*observers)[i]->update(loggable);
    }
}

// getter
const std::vector<Observer*>& Subject::getObservers() const {
    return *observers;
}

// Stream insertion operator for Subject.
std::ostream& operator<<(std::ostream& os, const Subject& subject) {
    os << "Subject with " << subject.observers->size() << " observer(s)";
    return os;
}

// =========== LogObserver ==========

//Starts a fresh log: the file is emptied once when the observer is created. 
LogObserver::LogObserver() {
    std::ofstream out(LOG_FILE_NAME, std::ios::trunc);
}


// Copy constructor. It must NOT empty the file, so it does not open it.
LogObserver::LogObserver(const LogObserver& other) : Observer(other) {
}
 
// Assignment operator. LogObserver has no data of its own to copy.
LogObserver& LogObserver::operator=(const LogObserver& other) {
    if (this != &other) {
        Observer::operator=(other);
    }
 
    return *this;
}
 
// Destructor.
LogObserver::~LogObserver() {
}

// Called by Subject::notify(); stringToLog() runs the version of the real class
void LogObserver::update(ILoggable* loggable) const{
    if (loggable == nullptr) {
        return;
    }
 
    // Appends the text of the loggable object to gamelog.txt. it gets the text to log from stringToLog() inherited from other classes
    std::ofstream out(LOG_FILE_NAME, std::ios::app);
    out << loggable->stringToLog() << std::endl;
}

std::ostream& operator<<(std::ostream& os, const LogObserver&) {
    os << "LogObserver writing to " << LOG_FILE_NAME;
    return os;
}
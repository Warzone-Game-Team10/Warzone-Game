#include "LoggingObserver.h"
#include <algorithm>
#include <fstream>

namespace {
    //filename to writ elogs into
    const char* const LOG_FILE_NAME = "gamelog.txt";
}

// ---------- ILoggable ----------

ILoggable::~ILoggable() {}

// ---------- Observer ----------

Observer::~Observer() {}

// ---------- Subject ----------

/** Creates a subject with an empty observer list. */
Subject::Subject() : observers(new std::vector<Observer*>()) {}

/** Deep copy: new vector, same (non-owned) observer pointers. */
Subject::Subject(const Subject& other)
    : observers(new std::vector<Observer*>(*other.observers)) {}

/** Deep copy assignment: reuses our own vector and copies the contents. */
Subject& Subject::operator=(const Subject& other) {
    if (this != &other) {
        *observers = *other.observers;
    }
    return *this;
}

/** Deletes the list only; the observers belong to whoever created them. */
Subject::~Subject() {
    delete observers;
}

/** Registers an observer so it gets notified of changes. */
void Subject::attach(Observer* observer) {
    if (observer == nullptr) return;
    // avoid attaching the same observer twice (it would log every line twice)
    if (std::find(observers->begin(), observers->end(), observer) == observers->end()) {
        observers->push_back(observer);
    }
}

/** Unregisters an observer. */
void Subject::detach(Observer* observer) {
    observers->erase(std::remove(observers->begin(), observers->end(), observer),
                     observers->end());
}

/** Calls update() on every attached observer. */
void Subject::notify(ILoggable* loggable) {
    for (Observer* observer : *observers) {
        observer->update(loggable);
    }
}

/** Read-only access to the list, used to pass observers to child objects. */
const std::vector<Observer*>& Subject::getObservers() const {
    return *observers;
}

std::ostream& operator<<(std::ostream& os, const Subject& subject) {
    os << "Subject with " << subject.observers->size() << " observer(s)";
    return os;
}

// ---------- LogObserver ----------

/** Starts a fresh log: the file is emptied once when the observer is created. */
LogObserver::LogObserver() {
    std::ofstream out(LOG_FILE_NAME, std::ios::trunc);
}

/** Copying must not erase the log, so nothing happens here. */
LogObserver::LogObserver(const LogObserver& other) : Observer(other) {}

LogObserver& LogObserver::operator=(const LogObserver& other) {
    Observer::operator=(other);
    return *this;
}

LogObserver::~LogObserver() {}

/** Appends the loggable's text to gamelog.txt. */
void LogObserver::update(ILoggable* loggable) {
    if (loggable == nullptr) return;
    std::ofstream out(LOG_FILE_NAME, std::ios::app);
    out << loggable->stringToLog() << std::endl;
}

std::ostream& operator<<(std::ostream& os, const LogObserver&) {
    os << "LogObserver writing to " << LOG_FILE_NAME;
    return os;
}
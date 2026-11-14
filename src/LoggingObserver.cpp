#include "LoggingObserver.h"

#include <algorithm>
#include <fstream>
#include <ostream>
#include <stdexcept>

namespace {
// All logging instances write to the assignment's required destination.
const char* const LOG_FILE_NAME = "gamelog.txt";
}

// Stateless interface special members are explicitly defined outside the header.
ILoggable::ILoggable() = default;
ILoggable::ILoggable(const ILoggable&) = default;
ILoggable& ILoggable::operator=(const ILoggable&) = default;
ILoggable::~ILoggable() = default;
std::ostream& operator<<(std::ostream& out, const ILoggable& object) {
    return out << object.stringToLog();
}

Observer::Observer() = default;
Observer::Observer(const Observer&) = default;
Observer& Observer::operator=(const Observer&) = default;
Observer::~Observer() = default;
std::ostream& operator<<(std::ostream& out, const Observer&) {
    return out << "Observer";
}

// Registration ownership is separate from observer lifetime.
Subject::Subject() : observers(new std::vector<Observer*>()) {}
Subject::Subject(const Subject& other)
    : observers(new std::vector<Observer*>(*other.observers)) {}
Subject& Subject::operator=(const Subject& other) {
    if (this != &other) {
        std::vector<Observer*> replacement(*other.observers);
        observers->swap(replacement);
    }
    return *this;
}
Subject::~Subject() { delete observers; }

void Subject::attach(Observer* observer) {
    if (observer && std::find(observers->begin(), observers->end(), observer) == observers->end())
        observers->push_back(observer);
}
void Subject::detach(Observer* observer) {
    observers->erase(std::remove(observers->begin(), observers->end(), observer), observers->end());
}
void Subject::notify(ILoggable* loggable) {
    if (!loggable) return;
    const auto snapshot = *observers;
    for (Observer* observer : snapshot) observer->update(loggable);
}
const std::vector<Observer*>& Subject::getObservers() const { return *observers; }
std::ostream& operator<<(std::ostream& out, const Subject& subject) {
    return out << "Subject with " << subject.observers->size() << " observer(s)";
}

// Opening or copying another observer must not erase earlier notifications.
LogObserver::LogObserver() {
    std::ofstream out(LOG_FILE_NAME, std::ios::app);
    if (!out) throw std::runtime_error("Cannot open gamelog.txt for logging.");
}
LogObserver::LogObserver(const LogObserver& other) : Observer(other) {}
LogObserver& LogObserver::operator=(const LogObserver& other) {
    if (this != &other) Observer::operator=(other);
    return *this;
}
LogObserver::~LogObserver() = default;

void LogObserver::resetLog() {
    std::ofstream out(LOG_FILE_NAME, std::ios::trunc);
    if (!out) throw std::runtime_error("Cannot reset gamelog.txt.");
}
void LogObserver::update(ILoggable* loggable) const {
    if (!loggable) return;
    std::ofstream out(LOG_FILE_NAME, std::ios::app);
    if (!out) throw std::runtime_error("Cannot open gamelog.txt for logging.");
    out << loggable->stringToLog() << std::endl;
    if (!out) throw std::runtime_error("Cannot write to gamelog.txt.");
}
std::ostream& operator<<(std::ostream& out, const LogObserver&) {
    return out << "LogObserver writing to " << LOG_FILE_NAME;
}

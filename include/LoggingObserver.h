#pragma once

#include <iosfwd>
#include <string>
#include <vector>

// Interface implemented by every class that can supply a log entry.
class ILoggable {
public:
    ILoggable();
    ILoggable(const ILoggable& other);
    ILoggable& operator=(const ILoggable& other);
    virtual ~ILoggable();
    // Return the current event's text; no file access belongs in subjects.
    virtual std::string stringToLog() const = 0;
    friend std::ostream& operator<<(std::ostream& out, const ILoggable& object);
};

// Interface for consumers notified when a subject changes.
class Observer {
public:
    Observer();
    Observer(const Observer& other);
    Observer& operator=(const Observer& other);
    virtual ~Observer();
    // The subject supplies an ILoggable view of the changed object.
    virtual void update(ILoggable* loggable) const = 0;
    friend std::ostream& operator<<(std::ostream& out, const Observer& observer);
};

// Owns an independent registration container, but borrows its observers.
// Applications must keep each observer alive until its subjects are destroyed
// or detach it first. Copies share observers, not the registration container.
class Subject {
public:
    Subject();
    Subject(const Subject& other);
    Subject& operator=(const Subject& other);
    virtual ~Subject();
    // Duplicate and null registrations are ignored.
    virtual void attach(Observer* observer);
    // Detaching never deletes the borrowed observer.
    virtual void detach(Observer* observer);
    // Dispatch the current event to each registered observer.
    void notify(ILoggable* loggable);
    // Borrow registrations so processors/lists can wire newly created children.
    const std::vector<Observer*>& getObservers() const;
    friend std::ostream& operator<<(std::ostream& out, const Subject& subject);
private:
    std::vector<Observer*>* observers;
};

// Appends subject-supplied entries to gamelog.txt in the current working folder.
class LogObserver : public Observer {
public:
    // Open without truncation; additional observers preserve current-game entries.
    LogObserver();
    LogObserver(const LogObserver& other);
    LogObserver& operator=(const LogObserver& other);
    ~LogObserver() override;
    // Explicitly start a fresh session before reading its first command.
    static void resetLog();
    // Append and flush one event, reporting file errors rather than hiding them.
    void update(ILoggable* loggable) const override;
    friend std::ostream& operator<<(std::ostream& out, const LogObserver& observer);
};

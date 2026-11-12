#ifndef OBSERVER_H
#define OBSERVER_H

#include <algorithm>
#include <vector>

// Observer pattern shared by the three parts (Tower, Map, Critter).
//
// Observable (the subject) keeps a list of observers and calls notify() every time one of its values changes.
// Observer is the interface of the views: update(subject) is called by notify(), and the view then reads the
// new values from the subject itself (pull model), so the subject never needs to know how it is displayed.
//
// Header only, so every program that already includes Map.h, Tower.h or Critter.h builds without new source files.

class Observable;

// Interface of every view that wants to be told when an Observable changes.
class Observer {
    public:
        virtual ~Observer() = default;

        // Called by Observable::notify() each time a value of the subject changed.
        virtual void update(Observable* subject) = 0;

        // Called when the subject is destroyed while this observer is still attached, so the observer can forget it
        // instead of keeping a dangling pointer. Does nothing by default.
        virtual void subjectDestroyed(Observable* subject) { (void)subject; }
};

// Base class of every class that can be observed (Tower, Map, Critter, CritterGroup).
class Observable {
    private:
        std::vector<Observer*> observers;

    protected:
        // Tells every attached observer that a value changed. Called by the subclasses after each change.
        void notify() {
            // iterate on a copy: an observer may detach itself (or another one) while it is being notified.
            std::vector<Observer*> current = observers;
            for (Observer* observer : current) {
                if (std::find(observers.begin(), observers.end(), observer) != observers.end()) {
                    observer->update(this);
                }
            }
        }

    public:
        Observable() = default;

        // A copy starts with no observers: the observers watch one object, not its values.
        Observable(const Observable&) {}

        // Assigning new values keeps this object's own observers (the copied object's observers are not taken).
        // The subclass is responsible for calling notify() when it assigns new values on purpose (see Map::loadFromFile).
        Observable& operator=(const Observable&) { return *this; }

        virtual ~Observable() {
            std::vector<Observer*> current = observers;
            observers.clear();
            for (Observer* observer : current) {
                observer->subjectDestroyed(this);
            }
        }

        // Plugs an observer. Attaching the same observer twice or a null pointer does nothing.
        void attach(Observer* observer) {
            if (observer != nullptr && std::find(observers.begin(), observers.end(), observer) == observers.end()) {
                observers.push_back(observer);
            }
        }

        // Unplugs an observer. Does nothing if it was not attached.
        void detach(Observer* observer) {
            observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
        }

        int getObserverCount() const {
            return static_cast<int>(observers.size());
        }
};

#endif

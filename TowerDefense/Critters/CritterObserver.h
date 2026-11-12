#ifndef CRITTEROBSERVER_H
#define CRITTEROBSERVER_H

#include "Observer.h"

#include <iostream>

class Critter;
class CritterGroup;

/// Part 3 of the observer assignment: the view of one critter.
/// It attaches itself to the critter when it is created and detaches when it is destroyed. Every time the critter
/// notifies a change (spawn, new cell, damage, slow, death, escape), update() re-displays the critter's view.
class CritterObserver : public Observer {
    private:
        Critter* critter;  // the observed critter, nullptr once the critter is destroyed
        std::ostream& out; // where the view is drawn (std::cout by default, a string stream in the tests)
        int refreshCount;

    public:
        explicit CritterObserver(Critter* critter, std::ostream& out = std::cout);
        ~CritterObserver() override;

        CritterObserver(const CritterObserver&) = delete;
        CritterObserver& operator=(const CritterObserver&) = delete;

        void update(Observable* subject) override;
        void subjectDestroyed(Observable* subject) override;

        /// Draws the critter's view now.
        void display();

        int getRefreshCount() const;
        const Critter* getCritter() const;
};

/// Part 3 of the observer assignment: the view of the critters of a wave.
/// Every time a critter is added to or removed from the group (and every game update in which a critter of the wave
/// spawns, changes cell, is hit, dies or escapes), update() re-displays the list of critters.
class CritterGroupObserver : public Observer {
    private:
        CritterGroup* group; // the observed group, nullptr once the group is destroyed
        std::ostream& out;
        int refreshCount;

    public:
        explicit CritterGroupObserver(CritterGroup* group, std::ostream& out = std::cout);
        ~CritterGroupObserver() override;

        CritterGroupObserver(const CritterGroupObserver&) = delete;
        CritterGroupObserver& operator=(const CritterGroupObserver&) = delete;

        void update(Observable* subject) override;
        void subjectDestroyed(Observable* subject) override;

        /// Draws the wave's view now.
        void display();

        int getRefreshCount() const;
        const CritterGroup* getGroup() const;
};

#endif

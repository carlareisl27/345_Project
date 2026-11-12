#ifndef TOWEROBSERVER_H
#define TOWEROBSERVER_H

#include "Observer.h"

#include <iostream>

class Tower;

/// Part 1 of the observer assignment: the view of one tower.
/// It attaches itself to the tower when it is created and detaches when it is destroyed. Every time the tower
/// notifies a change (purchase, upgrade, sell, move, shot), update() re-displays the tower's view.
class TowerObserver : public Observer {
    private:
        Tower* tower;      // the observed tower, nullptr once the tower is destroyed
        std::ostream& out; // where the view is drawn (std::cout by default, a string stream in the tests)
        int refreshCount;  // how many times the view was drawn

    public:
        explicit TowerObserver(Tower* tower, std::ostream& out = std::cout);
        ~TowerObserver() override;

        TowerObserver(const TowerObserver&) = delete;
        TowerObserver& operator=(const TowerObserver&) = delete;

        /// Called by Tower::notify(): re-displays the view.
        void update(Observable* subject) override;
        void subjectDestroyed(Observable* subject) override;

        /// Draws the tower's view now (used to show the first view, before anything changed).
        void display();

        int getRefreshCount() const;
        const Tower* getTower() const;
};

#endif

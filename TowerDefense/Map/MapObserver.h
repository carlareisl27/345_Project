#ifndef MAPOBSERVER_H
#define MAPOBSERVER_H

#include "Observer.h"

#include <iostream>

class Map;

/// Part 2 of the observer assignment: the view of a map.
/// It attaches itself to the map when it is created and detaches when it is destroyed. Every time the map notifies a
/// change (cell, entry, exit, tower placed / moved / removed, map loaded), update() re-displays the whole map.
class MapObserver : public Observer {
    private:
        Map* map;          // the observed map, nullptr once the map is destroyed
        std::ostream& out; // where the view is drawn (std::cout by default, a string stream in the tests)
        int refreshCount;  // how many times the view was drawn

    public:
        explicit MapObserver(Map* map, std::ostream& out = std::cout);
        ~MapObserver() override;

        MapObserver(const MapObserver&) = delete;
        MapObserver& operator=(const MapObserver&) = delete;

        /// Called by Map::notify(): re-displays the view.
        void update(Observable* subject) override;
        void subjectDestroyed(Observable* subject) override;

        /// Draws the map's view now (used to show the first view, before anything changed).
        void display();

        int getRefreshCount() const;
        const Map* getMap() const;
};

#endif

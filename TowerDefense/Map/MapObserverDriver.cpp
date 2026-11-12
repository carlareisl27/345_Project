// Part 2 of the observer assignment: Map observer driver.
// Creates a map, plugs a MapObserver to it, then changes values of the map (towers placed and moved, path changed,
// exit moved). Every change calls Map::notify(), which calls MapObserver::update(), which re-displays the map's view.
// The driver itself never calls display() after the first view: every view below is drawn by the observer pattern.

#include "Map.h"
#include "MapObserver.h"

#include <iostream>

using std::cout;

static void step(const char* text) {
    cout << ">>> " << text << "\n";
}

int main() {
    cout << "=== Part 2: Map observer ===\n\n";

    // the map is built before the observer is plugged, so building it draws nothing
    Map map(7, 7);
    map.setEntry(0, 0);
    for (int column = 1; column <= 5; column++) map.setCell(0, column, CellType::PATH);
    map.setCell(1, 5, CellType::PATH);
    for (int column = 1; column <= 5; column++) map.setCell(2, column, CellType::PATH);
    map.setCell(3, 1, CellType::PATH);
    for (int column = 1; column <= 5; column++) map.setCell(4, column, CellType::PATH);
    map.setCell(5, 5, CellType::PATH);
    map.setExit(6, 5);

    MapObserver view(&map); // the observer attaches itself to the map
    cout << "Map created and Map Observer plugged (" << map.getObserverCount() << " observer attached).\n";
    cout << "First view, drawn once by the driver:\n";
    view.display();

    step("place a Basic tower (B) on scenery cell (1, 2)");
    map.placeTower(1, 2, 'B');

    step("place a Splash tower (P) on scenery cell (3, 3)");
    map.placeTower(3, 3, 'P');

    step("move the Basic tower from (1, 2) to (1, 6)");
    map.moveTower(1, 2, 1, 6);

    step("try to move the Splash tower onto the path at (2, 3) (refused: nothing changes, so no new view)");
    bool moved = map.moveTower(3, 3, 2, 3);
    cout << "    move " << (moved ? "accepted" : "refused") << ", refresh count still " << view.getRefreshCount() << "\n\n";

    step("move the Splash tower from (3, 3) to (5, 4)");
    map.moveTower(3, 3, 5, 4);

    step("turn cell (3, 5) into path (a shortcut)");
    map.setCell(3, 5, CellType::PATH);

    step("move the exit to (6, 1): the path does not reach it yet, so the view shows an invalid map");
    map.setExit(6, 1);

    step("connect the new exit: cell (5, 1) becomes path, the map is valid again");
    map.setCell(5, 1, CellType::PATH);

    step("sell the Basic tower: it is removed from (1, 6)");
    map.removeTower(1, 6);

    cout << "The view was drawn " << view.getRefreshCount() << " times (1 by the driver, "
         << view.getRefreshCount() - 1 << " by the observer pattern).\n";
    return 0;
}

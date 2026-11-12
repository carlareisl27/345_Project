// Part 1 of the observer assignment: Tower observer driver.
// Creates a tower, plugs a TowerObserver to it, then changes values of the tower. Every change calls
// Tower::notify(), which calls TowerObserver::update(), which re-displays the tower's view.
// The driver itself never calls display() after the first view: every view below is drawn by the observer pattern.

#include "Tower.h"
#include "TowerObserver.h"
#include "Critter.h"
#include "Map.h"

#include <iostream>
#include <vector>

using std::cout;

// prints what the driver is about to change, so the output shows which change triggered which view
static void step(const char* text) {
    cout << ">>> " << text << "\n";
}

int main() {
    int coins = 1000;

    cout << "=== Part 1: Tower observer ===\n\n";

    SlowTower tower(2, 3);
    TowerObserver view(&tower); // the observer attaches itself to the tower

    cout << "Tower created and Tower Observer plugged (" << tower.getObserverCount() << " observer attached).\n";
    cout << "First view, drawn once by the driver:\n";
    view.display();

    step("purchase the tower (coins 1000)");
    tower.purchase(coins);

    step("upgrade to level 2");
    tower.upgrade(coins);

    step("upgrade to level 3");
    tower.upgrade(coins);

    step("try to upgrade past level 3 (refused: nothing changes, so no new view)");
    bool upgraded = tower.upgrade(coins);
    cout << "    upgrade " << (upgraded ? "accepted" : "refused") << ", refresh count still " << view.getRefreshCount() << "\n\n";

    step("move the tower to cell (1, 4)");
    tower.moveTo(1, 4);

    // a critter in range so the tower can shoot: the shot resets the cooldown
    Map map(3, 10);
    map.setEntry(0, 0);
    map.setExit(0, 9);
    for (int column = 1; column < 9; column++) {
        map.setCell(0, column, CellType::PATH);
    }
    Critter critter(CritterType::CANNON, 1);
    critter.spawn(map);
    critter.update(4.0 / critter.getSpeed()); // walks to (0, 4), next to the tower
    std::vector<Critter*> critters = {&critter};

    step("the tower shoots a critter in range (cooldown is reset)");
    tower.update(0.1, critters);
    cout << "    critter hit points after the shot: " << critter.getHitPoints() << "\n\n";

    step("sell the tower");
    tower.sell(coins);
    cout << "    coins after selling: " << coins << "\n\n";

    step("unplug the observer, then try to move the sold tower (refused, no view)");
    tower.detach(&view);
    tower.moveTo(0, 0);

    cout << "The view was drawn " << view.getRefreshCount() << " times (1 by the driver, "
         << view.getRefreshCount() - 1 << " by the observer pattern).\n";
    return 0;
}

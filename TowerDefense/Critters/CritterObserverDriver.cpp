// Part 3 of the observer assignment: Critter observer driver.
//  A) Creates a critter, plugs a CritterObserver to it and changes its values (spawn, walk, damage, slow, death).
//  B) Creates the critter group of a wave, plugs a CritterGroupObserver to it, then adds and removes critters.
// Every change calls notify() on the critter / group, which calls update() on the observer, which re-displays the view.
// The driver itself never calls display() after the first view of each part.

#include "Critter.h"
#include "CritterGroup.h"
#include "CritterGroupGenerator.h"
#include "CritterObserver.h"
#include "Map.h"

#include <iostream>

using std::cout;

static void step(const char* text) {
    cout << ">>> " << text << "\n";
}

// a straight 1 x 8 corridor: entry at column 0, exit at column 7
static Map makeCorridor() {
    Map map(3, 8);
    map.setEntry(1, 0);
    for (int column = 1; column < 7; column++) {
        map.setCell(1, column, CellType::PATH);
    }
    map.setExit(1, 7);
    return map;
}

int main() {
    Map map = makeCorridor();

    // ------------------------------------------------------------------ A) one critter
    cout << "=== Part 3 A: Critter observer (one critter) ===\n\n";

    Critter critter(CritterType::CANNON, 3);
    CritterObserver critterView(&critter); // the observer attaches itself to the critter
    cout << "Critter created and Critter Observer plugged (" << critter.getObserverCount() << " observer attached).\n";
    cout << "First view, drawn once by the driver:\n";
    critterView.display();

    step("the critter spawns on the entry");
    critter.spawn(map);

    step("the critter walks for 0.5 s: it stays in its cell, so nothing changes and there is no new view");
    critter.update(0.5);
    cout << "    refresh count still " << critterView.getRefreshCount() << "\n\n";

    step("the critter walks on: it enters the next cell");
    critter.update(1.0);

    step("a tower hits it for 25 damage");
    critter.takeDamage(25);

    step("a slow tower halves its speed for 2 s");
    critter.applySlow(0.5, 2.0);

    step("it walks for 2.5 s: it changes cell and the slow wears off");
    critter.update(2.5);

    step("a tower hits it for 100 damage: it dies");
    critter.takeDamage(100);

    step("it is hit again after dying (refused: nothing changes, so no new view)");
    critter.takeDamage(10);
    cout << "    refresh count still " << critterView.getRefreshCount() << "\n\n";

    // ------------------------------------------------------------------ B) critters added to / removed from a wave
    cout << "=== Part 3 B: Critter group observer (critters added and removed) ===\n\n";

    CritterGroupGenerator generator;
    CritterGroup wave = generator.generate(2, map);
    CritterGroupObserver waveView(&wave);
    cout << "Wave 2 created by the generator and Critter Group Observer plugged (" << wave.getObserverCount() << " observer attached).\n";
    cout << "First view, drawn once by the driver:\n";
    waveView.display();

    step("add a level 2 Cannon critter to the wave");
    wave.addCritter(Critter(CritterType::CANNON, 2));

    step("add a level 5 Super critter to the wave");
    wave.addCritter(Critter(CritterType::SUPER, 5));

    step("remove the first critter of the wave (index 0)");
    wave.removeCritter(0);

    step("remove the critter at index 2");
    wave.removeCritter(2);

    step("try to remove index 42 (refused: nothing changes, so no new view)");
    bool removed = wave.removeCritter(42);
    cout << "    remove " << (removed ? "accepted" : "refused") << ", refresh count still " << waveView.getRefreshCount() << "\n\n";

    step("start the wave: the first critter spawns on the entry");
    int coins = 100;
    wave.update(0.05, map, coins);

    step("remove the critter that is on the map (index 0)");
    wave.removeCritter(0);

    cout << "Critter view drawn " << critterView.getRefreshCount() << " times, critter group view drawn "
         << waveView.getRefreshCount() << " times (each time 1 by the driver, the rest by the observer pattern).\n";
    return 0;
}

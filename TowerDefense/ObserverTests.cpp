// Tests of the observer pattern (Tower, Map, Critter and CritterGroup observers).
// The views draw into a string stream, so the tests can count the refreshes and read what was drawn.

#include "Observer.h"
#include "TestHelper.h"

#include "Critter.h"
#include "CritterGroup.h"
#include "CritterGroupGenerator.h"
#include "CritterObserver.h"
#include "Map.h"
#include "MapObserver.h"
#include "Tower.h"
#include "TowerObserver.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// counts the notifications it receives, whatever the subject
class CountingObserver : public Observer {
    public:
        int updates = 0;
        int destroyed = 0;
        Observable* last = nullptr;

        void update(Observable* subject) override { updates++; last = subject; }
        void subjectDestroyed(Observable*) override { destroyed++; }
};

// detaches itself the first time it is notified
class DetachingObserver : public Observer {
    public:
        Observable* subject;
        int updates = 0;
        explicit DetachingObserver(Observable* subject) : subject(subject) {}
        void update(Observable* changed) override { updates++; changed->detach(this); }
};

static Map makeCorridor() {
    Map map(3, 8);
    map.setEntry(1, 0);
    for (int column = 1; column < 7; column++) {
        map.setCell(1, column, CellType::PATH);
    }
    map.setExit(1, 7);
    return map;
}

static void testObservable() {
    std::cout << "Observable base class\n";

    Map map(3, 3);
    CountingObserver first;
    CountingObserver second;

    map.attach(&first);
    map.attach(&first);
    map.attach(nullptr);
    check(map.getObserverCount() == 1, "attaching the same observer twice (or nullptr) keeps one observer");

    map.attach(&second);
    map.setCell(0, 0, CellType::PATH);
    check(first.updates == 1 && second.updates == 1, "notify reaches every attached observer");
    check(first.last == &map, "the observer receives the subject that changed");

    map.detach(&second);
    map.setCell(0, 1, CellType::PATH);
    check(first.updates == 2 && second.updates == 1, "a detached observer is not notified any more");

    Map copy = map;
    check(copy.getObserverCount() == 0, "a copy of an observable starts with no observers");
    Map other(2, 2);
    map = other;
    check(map.getObserverCount() == 1, "assigning new values keeps the object's own observers");

    DetachingObserver selfRemoving(&map);
    map.attach(&selfRemoving);
    map.setCell(0, 0, CellType::PATH); // the map is 2 x 2 now
    map.setCell(0, 1, CellType::PATH);
    check(selfRemoving.updates == 1 && map.getObserverCount() == 1, "an observer can detach itself while it is notified");

    CountingObserver watcher;
    {
        Map shortLived(2, 2);
        shortLived.attach(&watcher);
    }
    check(watcher.destroyed == 1, "observers are told when their subject is destroyed");
}

static void testTowerObserver() {
    std::cout << "\nPart 1: Tower observer\n";

    std::ostringstream screen;
    int coins = 1000;
    BasicTower tower(1, 1);
    TowerObserver view(&tower, screen);

    check(tower.getObserverCount() == 1, "TowerObserver attaches itself to the tower");
    check(view.getRefreshCount() == 0, "nothing is drawn before the tower changes");

    tower.purchase(coins);
    check(view.getRefreshCount() == 1, "purchase re-displays the tower view");
    check(screen.str().find("purchased") != std::string::npos, "the view shows the new status");

    tower.upgrade(coins);
    check(view.getRefreshCount() == 2, "upgrade re-displays the tower view");
    check(screen.str().find("2 / 3") != std::string::npos, "the view shows the new level");

    tower.upgrade(coins);
    int before = view.getRefreshCount();
    check(!tower.upgrade(coins) && view.getRefreshCount() == before, "a refused upgrade does not re-display the view");

    tower.moveTo(4, 5);
    check(view.getRefreshCount() == before + 1 && screen.str().find("(4, 5)") != std::string::npos, "moveTo re-displays the view with the new cell");
    tower.moveTo(4, 5);
    check(view.getRefreshCount() == before + 1, "moving onto the same cell is not a change");

    Map map = makeCorridor();
    Critter critter(CritterType::CANNON, 1);
    critter.spawn(map);
    critter.update(4.0 / critter.getSpeed());
    std::vector<Critter*> critters = {&critter};
    before = view.getRefreshCount();
    tower.update(0.1, critters);
    check(view.getRefreshCount() == before + 1, "a shot (cooldown reset) re-displays the view");
    tower.update(0.1, critters);
    check(view.getRefreshCount() == before + 1, "a tower waiting for its cooldown does not re-display the view");

    tower.sell(coins);
    check(view.getRefreshCount() == before + 2 && screen.str().find("sold") != std::string::npos, "sell re-displays the view");
    check(!tower.moveTo(0, 0), "a sold tower cannot be moved");

    std::unique_ptr<Tower> temporary = std::make_unique<SplashTower>(0, 0);
    std::ostringstream otherScreen;
    TowerObserver lateView(temporary.get(), otherScreen);
    temporary.reset();
    check(lateView.getTower() == nullptr, "the view forgets a destroyed tower (no dangling pointer)");
}

static void testMapObserver() {
    std::cout << "\nPart 2: Map observer\n";

    std::ostringstream screen;
    Map map = makeCorridor();
    MapObserver view(&map, screen);

    check(map.getObserverCount() == 1, "MapObserver attaches itself to the map");

    check(map.placeTower(0, 3, 'B') && view.getRefreshCount() == 1, "placing a tower re-displays the map view");
    check(map.hasTower(0, 3) && map.getTowerSymbol(0, 3) == 'B', "the map knows where the tower is");

    check(map.moveTower(0, 3, 2, 5) && view.getRefreshCount() == 2, "moving a tower re-displays the map view once");
    check(!map.hasTower(0, 3) && map.hasTower(2, 5), "the tower left its old cell and is on the new one");
    check(screen.str().find("B") != std::string::npos, "the view draws the tower letter");

    int before = view.getRefreshCount();
    check(!map.moveTower(2, 5, 1, 3), "a tower cannot be moved onto the path");
    check(!map.placeTower(1, 4), "a tower cannot be placed on the path");
    check(!map.placeTower(2, 5), "two towers cannot share a cell");
    check(!map.setCell(2, 5, CellType::PATH), "a cell with a tower cannot become path");
    check(!map.setEntry(2, 5) && !map.setExit(2, 5), "a cell with a tower cannot become the entry or the exit");
    check(view.getRefreshCount() == before, "refused changes do not re-display the view");

    map.setCell(0, 0, CellType::SCENERY);
    check(view.getRefreshCount() == before, "setting a cell to the type it already has is not a change");

    map.setCell(0, 1, CellType::PATH);
    check(view.getRefreshCount() == before + 1, "changing a cell re-displays the view");

    map.setExit(1, 6);
    check(view.getRefreshCount() == before + 2, "moving the exit re-displays the view");

    check(map.removeTower(2, 5) && view.getRefreshCount() == before + 3 && map.getTowerCount() == 0, "removing a tower re-displays the view");
    check(!map.removeTower(2, 5), "removing a tower twice is refused");

    const std::string file = "observer_test_map.txt";
    Map saved(4, 4);
    saved.setEntry(0, 0);
    saved.setCell(0, 1, CellType::PATH);
    saved.setExit(0, 2);
    if (saved.saveToFile(file)) {
        before = view.getRefreshCount();
        check(map.loadFromFile(file) && view.getRefreshCount() == before + 1, "loading a map re-displays the view once");
        check(map.getObserverCount() == 1 && map.getRows() == 4, "the observer is still plugged to the loaded map");
        std::remove(file.c_str());
    }
}

static void testCritterObserver() {
    std::cout << "\nPart 3: Critter observer\n";

    std::ostringstream screen;
    Map map = makeCorridor();
    Critter critter(CritterType::MELEE, 1);
    CritterObserver view(&critter, screen);

    check(critter.getObserverCount() == 1, "CritterObserver attaches itself to the critter");

    critter.spawn(map);
    check(view.getRefreshCount() == 1 && screen.str().find("ACTIVE") != std::string::npos, "spawning re-displays the critter view");

    critter.update(0.5);
    check(view.getRefreshCount() == 1, "moving inside the same cell does not re-display the view");
    critter.update(0.6);
    check(view.getRefreshCount() == 2 && critter.getColumn() == 1, "entering a new cell re-displays the view");

    critter.takeDamage(10);
    check(view.getRefreshCount() == 3 && screen.str().find("20 /  30") != std::string::npos, "damage re-displays the view with the new hit points");

    critter.applySlow(0.5, 1.0);
    check(view.getRefreshCount() == 4 && screen.str().find("slowed") != std::string::npos, "being slowed re-displays the view");

    critter.update(1.2); // 1.2 s at half speed for 1 s then full speed: 0.7 cells, same cell, but the slow wore off
    check(view.getRefreshCount() == 5 && critter.getSlowTimeLeft() == 0.0, "the slow wearing off re-displays the view");

    critter.takeDamage(100);
    check(view.getRefreshCount() == 6 && screen.str().find("DEAD") != std::string::npos, "dying re-displays the view");
    critter.takeDamage(1);
    check(view.getRefreshCount() == 6, "hitting a dead critter is not a change");

    Critter runner(CritterType::CASTER, 1);
    std::ostringstream runnerScreen;
    CritterObserver runnerView(&runner, runnerScreen);
    runner.spawn(map);
    runner.update(100.0);
    check(runner.hasEscaped() && runnerScreen.str().find("ESCAPED") != std::string::npos, "escaping re-displays the view");
}

static void testCritterGroupObserver() {
    std::cout << "\nPart 3: Critter group observer (critters added and removed)\n";

    std::ostringstream screen;
    Map map = makeCorridor();
    CritterGroupGenerator generator;
    CritterGroup group = generator.generate(1, map);
    CritterGroupObserver view(&group, screen);
    const int size = group.getSize();

    check(group.getObserverCount() == 1, "CritterGroupObserver attaches itself to the group");

    check(group.addCritter(Critter(CritterType::SUPER, 1)) && view.getRefreshCount() == 1, "adding a critter re-displays the view");
    check(group.getSize() == size + 1 && group.getCritters().back().getType() == CritterType::SUPER, "the critter is added at the end of the wave");
    check(screen.str().find("Super") != std::string::npos, "the view lists the new critter");

    check(group.removeCritter(0) && view.getRefreshCount() == 2 && group.getSize() == size, "removing a critter re-displays the view");
    check(!group.removeCritter(-1) && !group.removeCritter(100) && view.getRefreshCount() == 2, "removing a critter that does not exist is refused");

    Critter spawned(CritterType::MELEE, 1);
    spawned.spawn(map);
    check(!group.addCritter(spawned) && view.getRefreshCount() == 2, "a critter already on a map cannot be added");

    int coins = 0;
    group.update(0.05, map, coins);
    check(view.getRefreshCount() == 3 && group.getSpawnedCount() == 1, "a critter spawning re-displays the view");
    group.update(0.05, map, coins);
    check(view.getRefreshCount() == 3, "an update where nothing visible changed does not re-display the view");

    check(group.removeCritter(0) && group.getSpawnedCount() == 0, "removing a spawned critter also removes it from the spawned count");
    check(group.getActiveCritters().empty(), "no critter is left on the map after removing it");

    for (int i = 0; i < 2000 && !group.isFinished(); i++) {
        group.update(0.05, map, coins);
    }
    check(group.isFinished() && group.getEscapedCount() == group.getSize(), "the wave still plays to the end after adds and removes");
}

int main() {
    testObservable();
    testTowerObserver();
    testMapObserver();
    testCritterObserver();
    testCritterGroupObserver();
    return testSummary();
}

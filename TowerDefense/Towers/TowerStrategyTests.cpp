#include "Map.h"
#include "Tower.h"
#include "Critter.h"
#include "NearestToTowerStrategy.h"
#include "NearestToExitStrategy.h"
#include "StrongestCritterStrategy.h"
#include "WeakestCritterStrategy.h"
#include "TestHelper.h"

#include <iostream>
#include <memory>
#include <vector>

// Tests of Assignment 3, Part 1 (Strategy pattern): the four targeting strategies and how a Tower uses them.

static Map makeStraightMap() {
    Map map(3, 10);
    map.setEntry(1, 0);
    map.setExit(1, 9);
    for (int column = 1; column <= 8; column++) {
        map.setCell(1, column, CellType::PATH);
    }
    return map;
}

static void walk(Critter& critter, int cells) {
    for (int step = 0; step < 100000 && critter.getPathIndex() < cells; step++) {
        critter.update(0.01);
    }
}

// Scene shared by the tests. Tower at (0, 4), range 3.
//   A: cell (1, 2), 42 HP  -> strongest
//   B: cell (1, 4), 32 HP  -> nearest to the tower
//   C: cell (1, 6), 12 HP  -> weakest
//   D: cell (1, 7), 37 HP  -> nearest to the exit, out of the tower's range
struct Scene {
    Map map = makeStraightMap();
    Critter a{CritterType::MELEE, 3}, b{CritterType::MELEE, 3}, c{CritterType::MELEE, 3}, d{CritterType::MELEE, 3};
    std::vector<Critter*> all{&a, &b, &c, &d};

    Scene() {
        const int cells[] = {2, 4, 6, 7};
        const int damage[] = {0, 10, 30, 5};
        for (std::size_t i = 0; i < all.size(); i++) {
            all[i]->spawn(map);
            walk(*all[i], cells[i]);
            all[i]->takeDamage(damage[i]);
        }
    }
};

// counts the notifications of a tower
class CountingObserver : public Observer {
    public:
        int count = 0;
        void update(Observable*) override { count++; }
};

static void testSceneIsAsExpected(Scene& s) {
    std::cout << "Scene\n";
    check(s.a.getPathIndex() == 2 && s.b.getPathIndex() == 4 && s.c.getPathIndex() == 6 && s.d.getPathIndex() == 7,
          "critters walked 2, 4, 6 and 7 cells");
    check(s.a.getHitPoints() == 42 && s.b.getHitPoints() == 32 && s.c.getHitPoints() == 12 && s.d.getHitPoints() == 37,
          "critters have 42, 32, 12 and 37 hit points");
}

static void testEachStrategy(Scene& s) {
    std::cout << "\nEach strategy on a known candidate list\n";
    check(NearestToTowerStrategy(0, 4).selectTarget(s.all) == &s.b, "NearestToTowerStrategy picks B, the closest to the tower cell");
    check(NearestToExitStrategy().selectTarget(s.all) == &s.d, "NearestToExitStrategy picks D, the one that walked the most cells");
    check(StrongestCritterStrategy().selectTarget(s.all) == &s.a, "StrongestCritterStrategy picks A, the most hit points");
    check(WeakestCritterStrategy().selectTarget(s.all) == &s.c, "WeakestCritterStrategy picks C, the fewest hit points");

    BasicTower tower(0, 4);
    check(NearestToTowerStrategy(&tower).selectTarget(s.all) == &s.b, "NearestToTowerStrategy built from a tower uses the tower's cell");
    tower.moveTo(0, 7);
    check(NearestToTowerStrategy(&tower).selectTarget(s.all) == &s.d, "NearestToTowerStrategy follows the tower after moveTo");
}

static void testEmptyList() {
    std::cout << "\nEmpty candidate list\n";
    const std::vector<Critter*> none;
    check(NearestToTowerStrategy(0, 0).selectTarget(none) == nullptr, "NearestToTowerStrategy returns nullptr");
    check(NearestToExitStrategy().selectTarget(none) == nullptr, "NearestToExitStrategy returns nullptr");
    check(StrongestCritterStrategy().selectTarget(none) == nullptr, "StrongestCritterStrategy returns nullptr");
    check(WeakestCritterStrategy().selectTarget(none) == nullptr, "WeakestCritterStrategy returns nullptr");
}

static void testTieBreak(Scene& s) {
    std::cout << "\nTies keep the first candidate\n";
    // same hit points, same path index: the first one of the list wins, whatever the order
    Critter e(CritterType::MELEE, 3), f(CritterType::MELEE, 3);
    e.spawn(s.map);
    f.spawn(s.map);
    std::vector<Critter*> ef = {&e, &f};
    std::vector<Critter*> fe = {&f, &e};
    check(StrongestCritterStrategy().selectTarget(ef) == &e && StrongestCritterStrategy().selectTarget(fe) == &f, "StrongestCritterStrategy tie");
    check(WeakestCritterStrategy().selectTarget(ef) == &e && WeakestCritterStrategy().selectTarget(fe) == &f, "WeakestCritterStrategy tie");
    check(NearestToExitStrategy().selectTarget(ef) == &e && NearestToExitStrategy().selectTarget(fe) == &f, "NearestToExitStrategy tie");
    // A (1,2) and C (1,6) are both at distance sqrt(5) from (0,4)
    std::vector<Critter*> ac = {&s.a, &s.c};
    std::vector<Critter*> ca = {&s.c, &s.a};
    check(NearestToTowerStrategy(0, 4).selectTarget(ac) == &s.a && NearestToTowerStrategy(0, 4).selectTarget(ca) == &s.c, "NearestToTowerStrategy tie");
}

static void testTowerUsesStrategy(Scene& s) {
    std::cout << "\nTower holds and uses a strategy\n";
    BasicTower fresh(0, 4);
    BasicTower tower(0, 4);

    check(fresh.selectTarget(s.all) == &s.b, "a new tower targets the nearest critter (default NearestToTowerStrategy)");

    tower.setTargetingStrategy(std::make_unique<WeakestCritterStrategy>());
    check(tower.selectTarget(s.all) == &s.c, "after setTargetingStrategy(Weakest) the tower targets C");

    tower.setTargetingStrategy(std::make_unique<StrongestCritterStrategy>());
    check(tower.selectTarget(s.all) == &s.a, "after setTargetingStrategy(Strongest) the tower targets A");

    tower.setTargetingStrategy(std::make_unique<NearestToExitStrategy>());
    check(tower.selectTarget(s.all) == &s.c, "NearestToExit from the tower picks C: D is out of range and filtered out");

    tower.setTargetingStrategy(nullptr);
    check(tower.selectTarget(s.all) == fresh.selectTarget(s.all) && tower.selectTarget(s.all) == &s.b,
          "setTargetingStrategy(nullptr) behaves like a fresh tower (selects B)");

    // dead and null critters are never given to the strategy
    Critter dead(CritterType::CASTER, 1);
    dead.spawn(s.map);
    walk(dead, 5);
    dead.takeDamage(1000);
    std::vector<Critter*> withDeadAndNull = {nullptr, &dead, &s.c};
    tower.setTargetingStrategy(std::make_unique<WeakestCritterStrategy>());
    check(tower.selectTarget(withDeadAndNull) == &s.c, "null and dead critters are filtered out before the strategy");

    std::vector<Critter*> onlyFar = {&s.d};
    check(tower.selectTarget(onlyFar) == nullptr, "no critter in range: no target");

    // the observers are told about the change of strategy
    CountingObserver observer;
    tower.attach(&observer);
    tower.setTargetingStrategy(std::make_unique<StrongestCritterStrategy>());
    check(observer.count == 1, "setTargetingStrategy notifies the observers");
    tower.detach(&observer);

    // update() shoots the target chosen by the strategy
    int coins = 500;
    BasicTower shooter(0, 4);
    shooter.purchase(coins);
    shooter.setTargetingStrategy(std::make_unique<StrongestCritterStrategy>());
    const int hpA = s.a.getHitPoints();
    const int hpB = s.b.getHitPoints();
    shooter.update(1.0, s.all);
    check(s.a.getHitPoints() == hpA - shooter.getDamage() && s.b.getHitPoints() == hpB, "update() shoots the critter chosen by the strategy");
}

int main() {
    std::cout << "=== Tower strategy tests ===\n\n";
    Scene scene;
    testSceneIsAsExpected(scene);
    testEachStrategy(scene);
    testEmptyList();
    testTieBreak(scene);
    testTowerUsesStrategy(scene);
    return testSummary();
}

#include "Map.h"
#include "Tower.h"
#include "Critter.h"
#include "NearestToTowerStrategy.h"
#include "NearestToExitStrategy.h"
#include "StrongestCritterStrategy.h"
#include "WeakestCritterStrategy.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Driver for Assignment 3, Part 1 (Strategy pattern).
// One Basic Tower watches four critters walking on a straight path. The tower keeps the same critters in front of it
// while its targeting strategy is swapped four times with setTargetingStrategy; each strategy picks a different critter.

// straight path on the middle row: entry (1, 0) -> exit (1, 9)
static Map makeStraightMap() {
    Map map(3, 10);
    map.setEntry(1, 0);
    map.setExit(1, 9);
    for (int column = 1; column <= 8; column++) {
        map.setCell(1, column, CellType::PATH);
    }
    return map;
}

// moves a spawned critter forward until it has walked 'cells' path cells
static void walk(Critter& critter, int cells) {
    for (int step = 0; step < 100000 && critter.getPathIndex() < cells; step++) {
        critter.update(0.01);
    }
}

static std::string describe(const Critter* critter, const std::vector<Critter*>& critters, const char* const labels[]) {
    if (critter == nullptr) {
        return "none";
    }
    for (std::size_t i = 0; i < critters.size(); i++) {
        if (critters[i] == critter) {
            return std::string(labels[i]) + " at (" + std::to_string(critter->getRow()) + ", " + std::to_string(critter->getColumn())
                   + "), " + std::to_string(critter->getHitPoints()) + " HP, " + std::to_string(critter->getPathIndex()) + " cells walked";
        }
    }
    return "?";
}

int main() {
    std::cout << "=== Part 1: Tower targeting strategies (Strategy pattern) ===\n\n";

    Map map = makeStraightMap();

    // four level 3 melee critters (42 HP each), at different places on the path and with different hit points
    Critter a(CritterType::MELEE, 3), b(CritterType::MELEE, 3), c(CritterType::MELEE, 3), d(CritterType::MELEE, 3);
    std::vector<Critter*> critters = {&a, &b, &c, &d};
    const char* const labels[] = {"critter A", "critter B", "critter C", "critter D"};
    const int cellsWalked[] = {2, 4, 6, 7};
    const int damageTaken[] = {0, 10, 30, 5};

    for (std::size_t i = 0; i < critters.size(); i++) {
        critters[i]->spawn(map);
        walk(*critters[i], cellsWalked[i]);
        critters[i]->takeDamage(damageTaken[i]);
    }

    BasicTower tower(0, 4); // scenery cell above the path, range 3

    std::cout << "Map (E entry, X exit, . path, # scenery). The Basic Tower stands at (0, 4), range " << tower.getRange() << ":\n";
    map.displayMap();
    std::cout << "\nCritters on the path:\n";
    for (std::size_t i = 0; i < critters.size(); i++) {
        std::cout << "  " << describe(critters[i], critters, labels)
                  << (tower.isInRange(critters[i]->getRow(), critters[i]->getColumn()) ? "  (in range)" : "  (out of range)") << '\n';
    }
    std::cout << "\nThe tower only gives in-range critters to its strategy, so critter D is never a candidate from the tower.\n";
    std::cout << "Candidates given directly to each strategy: A, B, C, D.\n\n";

    struct Entry {
        const char* name;
        std::unique_ptr<TargetingStrategy> (*make)(const Tower&);
    };
    const Entry strategies[] = {
        {"NearestToTowerStrategy",   [](const Tower& t) -> std::unique_ptr<TargetingStrategy> { return std::make_unique<NearestToTowerStrategy>(&t); }},
        {"NearestToExitStrategy",    [](const Tower&) -> std::unique_ptr<TargetingStrategy> { return std::make_unique<NearestToExitStrategy>(); }},
        {"StrongestCritterStrategy", [](const Tower&) -> std::unique_ptr<TargetingStrategy> { return std::make_unique<StrongestCritterStrategy>(); }},
        {"WeakestCritterStrategy",   [](const Tower&) -> std::unique_ptr<TargetingStrategy> { return std::make_unique<WeakestCritterStrategy>(); }},
    };

    for (const Entry& entry : strategies) {
        std::unique_ptr<TargetingStrategy> standalone = entry.make(tower);
        Critter* fromStrategy = standalone->selectTarget(critters);

        tower.setTargetingStrategy(entry.make(tower));
        Critter* fromTower = tower.selectTarget(critters);

        std::cout << entry.name << ":\n"
                  << "  strategy alone, all four candidates -> " << describe(fromStrategy, critters, labels) << '\n'
                  << "  tower (in-range candidates only)    -> " << describe(fromTower, critters, labels) << '\n';
    }

    std::cout << "\nEmpty candidate list with WeakestCritterStrategy -> "
              << describe(WeakestCritterStrategy().selectTarget({}), critters, labels) << '\n';

    tower.setTargetingStrategy(nullptr);
    std::cout << "setTargetingStrategy(nullptr) restores the default: tower selects -> " << describe(tower.selectTarget(critters), critters, labels) << "\n\n";

    // the strategy also drives real shots: the tower shoots the weakest critter until it dies
    int coins = 500;
    tower.purchase(coins);
    tower.setTargetingStrategy(std::make_unique<WeakestCritterStrategy>());
    std::cout << "Tower bought and set to WeakestCritterStrategy, then it shoots twice:\n";
    for (int shot = 1; shot <= 2; shot++) {
        Critter* target = tower.selectTarget(critters);
        std::string before = describe(target, critters, labels);
        tower.update(1.0, critters);
        std::cout << "  shot " << shot << ": " << before << " -> " << target->getHitPoints() << " HP" << (target->isAlive() ? "" : " (killed)") << '\n';
    }

    return 0;
}

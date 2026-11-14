#include "Map.h"
#include "Tower.h"
#include "Critter.h"
#include "TowerDecorator.h"
#include "LevelUpDecorator.h"
#include "SplashDecorator.h"
#include "BurningDecorator.h"
#include "FreezingDecorator.h"
#include "TowerUpgrades.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

// Driver for Assignment 3, Part 3 (Decorator pattern).
// Upgrades are added to a Basic Tower at run time by wrapping it in decorators. The decorated tower is still used
// through the Tower interface (a std::unique_ptr<Tower>), like any tower of the game.

static void printStats(const char* title, const Tower& tower) {
    std::cout << "  " << std::left << std::setw(8) << title << std::setw(46) << tower.getName() << std::right
              << " damage " << std::setw(3) << tower.getDamage()
              << "  range " << std::fixed << std::setprecision(2) << tower.getRange()
              << "  fire rate " << tower.getFireRate()
              << "  refund " << tower.getRefundValue() << '\n';
}

// shows the tower before and after wrapping it
static void showWrapping(const char* what, Tower* decorated) {
    std::unique_ptr<Tower> tower(decorated); // the decorator owns the wrapped tower, the unique_ptr owns the decorator
    std::cout << what << ":\n";
    BasicTower plain(1, 1);
    printStats("before", plain);
    printStats("after", *tower);
    std::cout << '\n';
}

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

static void printCritters(const std::vector<Critter*>& critters, const char* const labels[]) {
    for (std::size_t i = 0; i < critters.size(); i++) {
        const Critter* c = critters[i];
        std::cout << "    " << labels[i] << " at (" << c->getRow() << ", " << c->getColumn() << "): " << std::setw(3) << c->getHitPoints()
                  << " HP" << (c->getSlowTimeLeft() > 0.0 ? ", slowed " : "");
        if (c->getSlowTimeLeft() > 0.0) std::cout << std::setprecision(2) << c->getSlowTimeLeft() << " s";
        std::cout << '\n';
    }
}

int main() {
    std::cout << "=== Part 3: Tower upgrades (Decorator pattern) ===\n\n";

    std::cout << "--- Each decorator alone on a Basic Tower ---\n\n";
    showWrapping("LevelUpDecorator (damage and range x1.2)", new LevelUpDecorator(new BasicTower(1, 1)));
    showWrapping("SplashDecorator (half damage to critters within 1.5 cells of the target)", new SplashDecorator(new BasicTower(1, 1)));
    showWrapping("BurningDecorator (3 ticks of 20% of the hit, one per second)", new BurningDecorator(new BasicTower(1, 1)));
    showWrapping("FreezingDecorator (half speed for 2 s)", new FreezingDecorator(new BasicTower(1, 1)));

    std::cout << "--- Stacked decorators ---\n\n";
    showWrapping("LevelUp twice (x1.44)", new LevelUpDecorator(new LevelUpDecorator(new BasicTower(1, 1))));

    // the upgrades bought one after the other during a game: each one wraps the tower built so far
    std::unique_ptr<Tower> tower = std::make_unique<BasicTower>(0, 2);
    std::cout << "Upgrading one tower step by step:\n";
    printStats("start", *tower);
    tower.reset(new LevelUpDecorator(tower.release()));
    printStats("+level", *tower);
    tower.reset(new SplashDecorator(tower.release()));
    printStats("+splash", *tower);
    tower.reset(new FreezingDecorator(tower.release()));
    printStats("+freeze", *tower);
    tower.reset(new BurningDecorator(tower.release(), 0.20, 3, 0.25)); // fast ticks so they show between two shots
    printStats("+burn", *tower);

    // Level Up during play: levelUp() pays the next level's cost and wraps the tower in a LevelUpDecorator,
    // which changes the level, the next level's cost, the refund, the range and the power
    std::cout << "\n--- Buying levels during play with levelUp() ---\n\n";
    int wallet = 1000;
    std::unique_ptr<Tower> played = std::make_unique<BasicTower>(0, 2);
    played->purchase(wallet);
    auto showLevel = [&](const char* step) {
        std::cout << "  " << std::left << std::setw(16) << step << std::setw(18) << played->getName() << std::right
                  << " level " << played->getLevel() << "/" << played->getMaxLevel()
                  << "  next level costs " << std::setw(3) << played->getUpgradeCost()
                  << "  refund " << std::setw(3) << played->getRefundValue()
                  << "  damage " << played->getDamage()
                  << "  range " << std::fixed << std::setprecision(2) << played->getRange()
                  << "  coins " << wallet << '\n';
    };
    showLevel("bought");
    levelUp(played, wallet);
    showLevel("levelUp()");
    levelUp(played, wallet);
    showLevel("levelUp()");
    std::cout << "  levelUp() at the maximum level: " << (levelUp(played, wallet) ? "accepted" : "refused") << '\n';
    std::cout << "  upgrade() on the decorated tower: " << (played->upgrade(wallet) ? "accepted" : "refused")
              << " (levels are only added by decorators)\n";
    addBurning(played, wallet);
    showLevel("addBurning()");
    std::cout << "  sell: +" << played->sell(wallet) << " coins (half of the tower and of both levels)\n";

    // the decorated tower in action
    std::cout << "\n--- The fully upgraded tower in action ---\n\n";
    Map map = makeStraightMap();
    map.displayMap();

    Critter target(CritterType::MELEE, 10), neighbour(CritterType::MELEE, 10), far(CritterType::MELEE, 10);
    std::vector<Critter*> critters = {&target, &neighbour, &far};
    const char* const labels[] = {"target   ", "neighbour", "far      "};
    const int cells[] = {2, 3, 7};
    for (std::size_t i = 0; i < critters.size(); i++) {
        critters[i]->spawn(map);
        walk(*critters[i], cells[i]);
    }

    int coins = 500;
    tower->purchase(coins);
    std::cout << "\nTower " << tower->getName() << " bought at (" << tower->getRow() << ", " << tower->getColumn() << ").\n";
    std::cout << "Critters (level 10 melee):\n";
    printCritters(critters, labels);

    tower->update(0.0, critters);
    std::cout << "\nt = 0.00 s, the tower shoots the nearest critter:\n"
              << "  target -24 (Lv2 damage), neighbour -12 (splash: half of the Lv2 damage), far untouched,\n"
                 "  target slowed (freezing) and burning (4 per tick = 20% of 24)\n";
    printCritters(critters, labels);

    for (double t = 0.25; t < 0.99; t += 0.25) {
        tower->update(0.25, critters);
        std::cout << "t = " << std::fixed << std::setprecision(2) << t << " s, burning tick (-4):\n";
        printCritters(critters, labels);
    }

    tower->update(0.25, critters);
    std::cout << "t = 1.00 s, cooldown over, second shot:\n";
    printCritters(critters, labels);

    return 0;
}

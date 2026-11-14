#include "Map.h"
#include "Tower.h"
#include "Critter.h"
#include "CritterGroup.h"
#include "CritterFactory.h"
#include "StandardCritterFactory.h"
#include "NearestToExitStrategy.h"
#include "StrongestCritterStrategy.h"
#include "TowerDecorator.h"
#include "TowerUpgrades.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

// Integrated driver: Part 1 (map), Part 2 (towers) and Part 3 (critters) working together.
// The player buys towers on scenery cells, then plays waves. Towers shoot the critters, kills give coins,
// critters that reach the exit steal coins, and between waves the player upgrades towers with the coins left.
//
// Assignment 3 patterns used by the game:
// - Strategy: each tower has a targeting strategy (the splash tower aims at the strongest critter, the slow tower at
//   the critter closest to the exit, the basic tower at the nearest critter).
// - Factory: every wave is created by a CritterFactory, which makes later waves larger and stronger.
// - Decorator: every upgrade wraps the tower in a decorator: a LevelUpDecorator per level, then, at the maximum level,
//   one special upgrade (Burning, Freezing or Splash).

// Decorator pattern: a tower at its maximum level buys the special upgrade that fits its kind, through the helpers of
// TowerUpgrades.h (each one checks the price, takes the coins and wraps the tower in the decorator). The unique_ptr then
// owns the decorator, which owns the original tower. False if the upgrade is already there or too expensive.
bool addSpecialUpgrade(std::unique_ptr<Tower>& tower, int& coins) {
    const Tower& base = baseTower(*tower);
    if (dynamic_cast<const BasicTower*>(&base) != nullptr) {
        return addBurning(tower, coins);  // the basic tower sets its target on fire
    }
    if (dynamic_cast<const SplashTower*>(&base) != nullptr) {
        return addFreezing(tower, coins); // the splash tower slows the whole group it hits
    }
    return addSplash(tower, coins);       // the slow tower slows its target and damages the critters around it
}

// a tower can only be bought if it stands on a scenery cell and the player can pay for it.
bool buyTower(const Map& map, Tower& tower, int& coins) {
    if (!map.isSceneryCell(tower.getRow(), tower.getColumn())) {
        std::cout << "  " << tower.getName() << " at (" << tower.getRow() << ", " << tower.getColumn() << ") refused: not a scenery cell\n";
        return false;
    }
    if (!tower.purchase(coins)) {
        std::cout << "  " << tower.getName() << " at (" << tower.getRow() << ", " << tower.getColumn() << ") refused: not enough coins\n";
        return false;
    }
    std::cout << "  bought " << tower.getName() << " at (" << tower.getRow() << ", " << tower.getColumn() << "), coins left: " << coins << '\n';
    return true;
}

// map in the console: E entry, X exit, . path, # scenery, B/P/L towers (basic, splash, slow) and one letter per critter.
void drawMap(const Map& map, const CritterGroup& group, const std::vector<std::unique_ptr<Tower>>& towers) {
    for (int row = 0; row < map.getRows(); row++) {
        for (int column = 0; column < map.getColumns(); column++) {
            char symbol = '#';
            if (map.getEntry() == std::make_pair(row, column)) symbol = 'E';
            else if (map.getExit() == std::make_pair(row, column)) symbol = 'X';
            else if (map.isPathCell(row, column)) symbol = '.';

            for (const auto& tower : towers) {
                if (tower->isPurchased() && tower->getRow() == row && tower->getColumn() == column) {
                    symbol = tower->getName()[0] == 'B' ? 'B' : (tower->getName()[0] == 'S' && tower->getName()[1] == 'p' ? 'P' : 'L');
                }
            }
            for (const Critter& critter : group.getCritters()) {
                if (critter.isActive() && critter.getRow() == row && critter.getColumn() == column) {
                    symbol = critter.getSymbol();
                    break;
                }
            }
            std::cout << symbol << ' ';
        }
        std::cout << '\n';
    }
}

void playWave(const Map& map, CritterGroup& group, std::vector<std::unique_ptr<Tower>>& towers, int& coins, bool showMap) {
    const double deltaTime = 0.1;
    double time = 0.0;
    bool shown = false;

    while (!group.isFinished() && time < 120.0) {
        for (auto& tower : towers) {
            tower->update(deltaTime, group.getActiveCritters());
        }
        group.update(deltaTime, map, coins);
        time += deltaTime;

        if (showMap && !shown && time >= 9.0) {
            std::cout << "\nMap at t=" << std::fixed << std::setprecision(1) << time << "s (B basic, P splash, L slow tower):\n";
            drawMap(map, group, towers);
            std::cout << '\n';
            shown = true;
        }
    }

    std::cout << "Wave " << group.getWaveNumber() << " (" << group.getSize() << " critters, one every " << std::fixed << std::setprecision(2)
              << group.getSpawnInterval() << " s): " << group.getKilledCount() << " killed (+" << group.getCoinsEarned() << " coins), "
              << group.getEscapedCount() << " escaped (-" << group.getCoinsStolen() << " coins), coins now " << coins << '\n';
}

// between waves the player first buys the towers it could not afford before, then buys the special upgrade of towers
// already at their maximum level, then upgrades with the coins left.
void spendCoins(const Map& map, std::vector<std::unique_ptr<Tower>>& towers, int& coins) {
    for (auto& tower : towers) {
        if (!tower->isPurchased() && !tower->isSold() && map.isSceneryCell(tower->getRow(), tower->getColumn()) && tower->purchase(coins)) {
            std::cout << "  bought " << tower->getName() << " at (" << tower->getRow() << ", " << tower->getColumn() << "), coins left: " << coins << '\n';
        }
    }
    // a tower already at its maximum level gets its special upgrade first (Decorator pattern)
    for (auto& tower : towers) {
        if (tower->isPurchased() && tower->getLevel() == tower->getMaxLevel() && addSpecialUpgrade(tower, coins)) {
            std::cout << "  special upgrade: now " << tower->getName() << " (damage " << tower->getDamage() << ", range "
                      << tower->getRange() << "), coins left: " << coins << '\n';
        }
    }
    // Level Up (Decorator pattern): each upgrade wraps the tower in one more LevelUpDecorator
    for (auto& tower : towers) {
        if (tower->isPurchased() && levelUp(tower, coins)) {
            std::cout << "  upgraded " << tower->getName() << " to level " << tower->getLevel() << ", coins left: " << coins << '\n';
        }
    }
}

int main() {
    Map map(7, 7);
    map.setEntry(0, 0);
    map.setExit(6, 5);
    for (int column = 1; column <= 5; column++) map.setCell(0, column, CellType::PATH);
    map.setCell(1, 5, CellType::PATH);
    for (int column = 1; column <= 5; column++) map.setCell(2, column, CellType::PATH);
    map.setCell(3, 1, CellType::PATH);
    for (int column = 1; column <= 5; column++) map.setCell(4, column, CellType::PATH);
    map.setCell(5, 5, CellType::PATH);

    std::cout << "=== Integrated driver: map + towers + critters ===\n\nMap (E entry, X exit, . path, # scenery):\n";
    map.displayMap();
    std::cout << (map.isValidMap() ? "The map is valid.\n\n" : "The map is invalid.\n\n");

    int coins = 160;
    std::vector<std::unique_ptr<Tower>> towers;
    towers.push_back(std::make_unique<BasicTower>(2, 2)); // on the path: must be refused
    towers.push_back(std::make_unique<BasicTower>(3, 3));
    towers.push_back(std::make_unique<SplashTower>(3, 4));
    towers.push_back(std::make_unique<SlowTower>(1, 3));

    // Strategy pattern: each tower gets the targeting that suits it (the basic tower keeps the default, nearest critter)
    towers[2]->setTargetingStrategy(std::make_unique<StrongestCritterStrategy>()); // splash where the toughest critter is
    towers[3]->setTargetingStrategy(std::make_unique<NearestToExitStrategy>());    // slow the critter about to escape

    std::cout << "Buying towers with " << coins << " coins:\n";
    for (auto& tower : towers) {
        buyTower(map, *tower, coins);
    }
    std::cout << '\n';

    // Factory pattern: the game only knows the CritterFactory interface
    StandardCritterFactory standardWaves;
    const CritterFactory& waveFactory = standardWaves;
    for (int wave = 1; wave <= 10; wave++) {
        CritterGroup group = waveFactory.createGroup(wave, map);
        playWave(map, group, towers, coins, wave == 3);

        if (wave < 10) {
            spendCoins(map, towers, coins);
        }
    }

    int total = 0;
    for (const auto& tower : towers) {
        if (tower->isPurchased()) {
            std::cout << tower->getName() << " level " << tower->getLevel() << " would be sold for " << tower->getRefundValue() << " coins\n";
            total += tower->sell(coins);
        }
    }
    std::cout << "Selling every tower gives " << total << " coins, final coins: " << coins << '\n';

    return 0;
}

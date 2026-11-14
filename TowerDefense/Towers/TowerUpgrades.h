#ifndef TOWERUPGRADES_H
#define TOWERUPGRADES_H

#include "Tower.h"
#include "TowerDecorator.h"

#include <memory>

/// Decorator pattern (Assignment 3, Part 3): the upgrades the player buys during a game.
///
/// Each helper checks the rules and the price, takes the coins, then wraps the tower in a new decorator:
/// tower.reset(new SomeDecorator(tower.release())). The unique_ptr then owns the decorator, which owns the old tower,
/// so the game and the GUI keep using the same std::unique_ptr<Tower> slot. Each helper returns false (and leaves the
/// tower and the coins unchanged) when the upgrade is refused.

/// Price of the special upgrades, which can be bought once each, at any level.
constexpr int SPLASH_UPGRADE_COST = 150;
constexpr int BURNING_UPGRADE_COST = 125;
constexpr int FREEZING_UPGRADE_COST = 100;

/// The concrete tower at the bottom of a decorator chain (the tower itself if it is not decorated).
const Tower& baseTower(const Tower& tower);

/// The first decorator of type D found in the chain, from the outermost one inward, or nullptr.
template <typename D>
const D* findDecorator(const Tower& tower) {
    const Tower* current = &tower;
    while (current != nullptr) {
        if (const D* found = dynamic_cast<const D*>(current)) {
            return found;
        }
        const TowerDecorator* decorator = dynamic_cast<const TowerDecorator*>(current);
        current = decorator != nullptr ? decorator->getWrappedTower() : nullptr;
    }
    return nullptr;
}

/// True if the chain contains a decorator of type D.
template <typename D>
bool hasDecorator(const Tower& tower) {
    return findDecorator<D>(tower) != nullptr;
}

/// Level Up: refused if the tower is null, not purchased, sold, already at its maximum level, or if coins is less than
/// tower->getUpgradeCost(). Otherwise takes the cost and wraps the tower in a LevelUpDecorator that remembers the price.
bool levelUp(std::unique_ptr<Tower>& tower, int& coins);

/// Special upgrades: refused if the tower is null, not purchased, sold, already has that upgrade, or the player cannot
/// pay its fixed price. Otherwise the price is taken and the tower is wrapped in the decorator.
bool addSplash(std::unique_ptr<Tower>& tower, int& coins);
bool addBurning(std::unique_ptr<Tower>& tower, int& coins);
bool addFreezing(std::unique_ptr<Tower>& tower, int& coins);

/// True if the special upgrade can be bought now (same checks as the helpers, without buying anything).
bool canAddSplash(const Tower& tower, int coins);
bool canAddBurning(const Tower& tower, int coins);
bool canAddFreezing(const Tower& tower, int coins);

#endif

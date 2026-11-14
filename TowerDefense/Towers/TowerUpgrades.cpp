// Decorator pattern (Assignment 3, Part 3): the upgrades the player buys during a game. Each helper checks the rules
// and the price, takes the coins, then wraps the tower in a new decorator inside the same unique_ptr.
#include "TowerUpgrades.h"
#include "LevelUpDecorator.h"
#include "SplashDecorator.h"
#include "BurningDecorator.h"
#include "FreezingDecorator.h"

// follows getWrappedTower() until the object is not a decorator: that is the concrete tower (Basic, Splash, Slow)
const Tower& baseTower(const Tower& tower) {
    const Tower* current = &tower;
    const TowerDecorator* decorator = dynamic_cast<const TowerDecorator*>(current);
    while (decorator != nullptr) {
        current = decorator->getWrappedTower();
        decorator = dynamic_cast<const TowerDecorator*>(current);
    }
    return *current;
}

namespace {

// upgrades are only sold for a tower that is on the map: bought and not sold
bool isActiveTower(const Tower& tower) {
    return tower.isPurchased() && !tower.isSold();
}

// a special upgrade D can be bought once per tower, if the player can pay for it
template <typename D>
bool canAdd(const Tower& tower, int coins, int price) {
    return isActiveTower(tower) && !hasDecorator<D>(tower) && coins >= price;
}

// takes the price and wraps the tower in a new D (built by 'wrap'), if canAdd allows it
template <typename D, typename Wrap>
bool addSpecial(std::unique_ptr<Tower>& tower, int& coins, int price, Wrap wrap) {
    if (tower == nullptr || !canAdd<D>(*tower, coins, price)) {
        return false;
    }
    coins -= price;
    tower.reset(wrap(tower.release()));
    return true;
}

} // namespace

// Level Up: the price is the tower's next level cost (cost x current level), remembered by the decorator for the refund
bool levelUp(std::unique_ptr<Tower>& tower, int& coins) {
    if (tower == nullptr || !isActiveTower(*tower) || tower->getLevel() >= tower->getMaxLevel()) {
        return false;
    }

    const int cost = tower->getUpgradeCost();
    if (cost <= 0 || coins < cost) {
        return false;
    }

    coins -= cost;
    tower.reset(new LevelUpDecorator(tower.release(), cost));
    return true;
}

// ---- the three special upgrades, each with its fixed price ----

bool canAddSplash(const Tower& tower, int coins) {
    return canAdd<SplashDecorator>(tower, coins, SPLASH_UPGRADE_COST);
}

bool canAddBurning(const Tower& tower, int coins) {
    return canAdd<BurningDecorator>(tower, coins, BURNING_UPGRADE_COST);
}

bool canAddFreezing(const Tower& tower, int coins) {
    return canAdd<FreezingDecorator>(tower, coins, FREEZING_UPGRADE_COST);
}

bool addSplash(std::unique_ptr<Tower>& tower, int& coins) {
    return addSpecial<SplashDecorator>(tower, coins, SPLASH_UPGRADE_COST,
                                       [](Tower* plain) { return new SplashDecorator(plain); });
}

bool addBurning(std::unique_ptr<Tower>& tower, int& coins) {
    return addSpecial<BurningDecorator>(tower, coins, BURNING_UPGRADE_COST,
                                        [](Tower* plain) { return new BurningDecorator(plain); });
}

bool addFreezing(std::unique_ptr<Tower>& tower, int& coins) {
    return addSpecial<FreezingDecorator>(tower, coins, FREEZING_UPGRADE_COST,
                                         [](Tower* plain) { return new FreezingDecorator(plain); });
}

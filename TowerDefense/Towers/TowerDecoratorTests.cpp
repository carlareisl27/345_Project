#include "Map.h"
#include "Tower.h"
#include "Critter.h"
#include "TowerDecorator.h"
#include "LevelUpDecorator.h"
#include "SplashDecorator.h"
#include "BurningDecorator.h"
#include "FreezingDecorator.h"
#include "WeakestCritterStrategy.h"
#include "TowerUpgrades.h"
#include "TestHelper.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// Tests of Assignment 3, Part 3 (Decorator pattern).

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

// a decorator that adds nothing: used to check that TowerDecorator forwards everything
class PlainDecorator : public TowerDecorator {
    public:
        explicit PlainDecorator(Tower* tower) : TowerDecorator(tower) {}
        const char* getDecoratorLabel() const override { return "Plain"; }
};

// a Basic Tower that counts how many times it was destroyed (ownership test)
class CountedTower : public BasicTower {
    public:
        static int destroyed;
        CountedTower(int row, int column) : BasicTower(row, column) {}
        ~CountedTower() override { destroyed++; }
};
int CountedTower::destroyed = 0;

class CountingObserver : public Observer {
    public:
        int count = 0;
        void update(Observable*) override { count++; }
};

// Scene: tower at (0, 2), level 10 melee critters (84 HP) at (1, 2) = target, (1, 3) = neighbour, (1, 7) = far.
struct Scene {
    Map map = makeStraightMap();
    Critter target{CritterType::MELEE, 10}, neighbour{CritterType::MELEE, 10}, far{CritterType::MELEE, 10};
    std::vector<Critter*> all{&target, &neighbour, &far};

    Scene() {
        const int cells[] = {2, 3, 7};
        for (std::size_t i = 0; i < all.size(); i++) {
            all[i]->spawn(map);
            walk(*all[i], cells[i]);
        }
    }
};

static void testLevelUp() {
    std::cout << "LevelUpDecorator\n";
    BasicTower basic(1, 1);
    std::unique_ptr<Tower> once = std::make_unique<LevelUpDecorator>(new BasicTower(1, 1));
    std::unique_ptr<Tower> twice = std::make_unique<LevelUpDecorator>(new LevelUpDecorator(new BasicTower(1, 1)));

    check(once->getDamage() > basic.getDamage() && once->getRange() > basic.getRange(),
          "one LevelUpDecorator: damage and range strictly greater than the Basic Tower");
    check(twice->getDamage() > once->getDamage(), "two stacked LevelUpDecorators: damage strictly greater than one");
    check(once->getDamage() == 24 && std::fabs(once->getRange() - 3.6) < 1e-9, "one level: damage 20 -> 24, range 3 -> 3.6 (x1.2)");
    check(twice->getDamage() == 28 && std::fabs(twice->getRange() - 4.32) < 1e-9, "two levels: damage 28 (24 x 1.2 rounded down), range 4.32 (x1.44)");
    check(once->getRefundValue() == basic.getRefundValue(), "the refund value is the wrapped tower's");
    check(std::string(once->getName()) == "Basic Tower Lv2" && std::string(twice->getName()) == "Basic Tower Lv3",
          "names: \"Basic Tower Lv2\" and \"Basic Tower Lv3\"");
}

static void testForwarding() {
    std::cout << "\nTowerDecorator forwards everything\n";
    BasicTower reference(2, 3);
    PlainDecorator plain(new BasicTower(2, 3));

    check(std::string(plain.getName()) == "Basic Tower +Plain", "name: base name followed by the decorator label");
    check(plain.getDamage() == reference.getDamage() && plain.getRange() == reference.getRange()
              && plain.getFireRate() == reference.getFireRate() && plain.getCost() == reference.getCost()
              && plain.getRefundValue() == reference.getRefundValue() && plain.getLevel() == reference.getLevel()
              && plain.getMaxLevel() == reference.getMaxLevel() && plain.getRow() == 2 && plain.getColumn() == 3,
          "getters equal to the wrapped tower's");

    int coins = 1000;
    CountingObserver observer;
    plain.attach(&observer);
    check(plain.purchase(coins) && plain.isPurchased() && plain.getWrappedTower()->isPurchased() && coins == 900,
          "purchase is forwarded to the wrapped tower");
    check(plain.upgrade(coins) && plain.getLevel() == 2 && plain.getDamage() == 25, "upgrade is forwarded (level 2, damage 25)");
    check(plain.moveTo(0, 0) && plain.getRow() == 0 && plain.getColumn() == 0, "moveTo is forwarded");
    check(plain.sell(coins) > 0 && plain.isSold(), "sell is forwarded");
    check(observer.count == 4, "each forwarded change notifies the decorator's observers");
    plain.detach(&observer);

    bool thrown = false;
    try {
        PlainDecorator broken(nullptr);
    } catch (const std::invalid_argument&) {
        thrown = true;
    }
    check(thrown, "wrapping nullptr is refused");

    CountedTower::destroyed = 0;
    {
        LevelUpDecorator outer(new SplashDecorator(new CountedTower(0, 0)));
    }
    check(CountedTower::destroyed == 1, "destroying the outer decorator destroys the wrapped chain once");
}

static void testRange() {
    std::cout << "\nRange of a decorated tower\n";
    BasicTower basic(0, 0);
    LevelUpDecorator decorated(new BasicTower(0, 0)); // range 3.6

    check(!basic.isInRange(1, 3), "(1, 3) is at 3.16 cells: out of the Basic Tower's range 3");
    check(decorated.isInRange(1, 3), "isInRange is true for (1, 3), inside the decorator's range 3.6");
    check(!decorated.isInRange(0, 4), "isInRange is false for (0, 4), beyond the decorator's range 3.6");
}

static void testNames() {
    std::cout << "\nNames of decorated towers\n";
    std::unique_ptr<Tower> tower = std::make_unique<BurningDecorator>(
        new FreezingDecorator(new SplashDecorator(new LevelUpDecorator(new BasicTower(0, 0)))));
    const std::string name = tower->getName();
    check(name.find("Basic Tower") != std::string::npos && name.find("Lv2") != std::string::npos && name.find("Splash") != std::string::npos
              && name.find("Freezing") != std::string::npos && name.find("Burning") != std::string::npos,
          "the name contains the base name and every decorator label");
    check(name == "Basic Tower Lv2 +Splash +Freezing +Burning", "labels appear in the order the upgrades were applied");
}

static void testSplash() {
    std::cout << "\nSplashDecorator\n";
    Scene s;
    int coins = 500;
    SplashDecorator tower(new BasicTower(0, 2));
    check(tower.getSplashRadius() == 1.5 && tower.getSplashDamage() == 10, "default radius 1.5, splash damage 10 (half of 20)");
    tower.purchase(coins);
    check(tower.update(0.0, s.all), "the decorated tower shoots");
    check(s.target.getHitPoints() == 84 - 20, "target takes the full 20");
    check(s.neighbour.getHitPoints() == 84 - 10, "neighbour (1 cell from the target) takes 10");
    check(s.far.getHitPoints() == 84, "far critter untouched");

    Scene killed;
    killed.target.takeDamage(80); // 4 HP left: the hit kills it
    SplashDecorator finisher(new BasicTower(0, 2));
    finisher.purchase(coins);
    finisher.update(0.0, killed.all);
    check(!killed.target.isAlive() && killed.neighbour.getHitPoints() == 84 - 10, "the splash still happens when the target dies");
}

static void testBurning() {
    std::cout << "\nBurningDecorator\n";
    Scene s;
    int coins = 500;
    BurningDecorator tower(new BasicTower(0, 2), 0.20, 3, 0.25); // ticks every 0.25 s, the Basic Tower shoots every 1 s
    tower.purchase(coins);

    tower.update(0.0, s.all);
    check(s.target.getHitPoints() == 64 && tower.isBurning(&s.target), "hit for 20, the target is burning");
    check(tower.getBurnTickDamage(&s.target) == 4 && tower.getBurnTicksLeft(&s.target) == 3, "burn: 3 ticks of 4 (20% of 20)");

    tower.update(0.125, s.all);
    check(s.target.getHitPoints() == 64, "no damage before the tick interval");
    tower.update(0.125, s.all);
    check(s.target.getHitPoints() == 60 && tower.getBurnTicksLeft(&s.target) == 2, "first tick after 0.25 s: -4");
    tower.update(0.25, s.all);
    tower.update(0.25, s.all);
    check(s.target.getHitPoints() == 52 && !tower.isBurning(&s.target), "after 3 ticks the burn is over (52 HP)");

    tower.update(0.25, s.all); // t = 1.0: second shot, new burn
    check(s.target.getHitPoints() == 32 && tower.getBurnTicksLeft(&s.target) == 3, "second shot starts a new burn");
    tower.update(0.25, s.all);
    check(tower.getBurnTicksLeft(&s.target) == 2, "one tick later: 2 ticks left");

    // a new hit on a burning critter restarts the ticks without a second burn
    BurningDecorator fast(new BasicTower(0, 2), 0.20, 3, 0.25);
    Scene s2;
    fast.purchase(coins);
    fast.update(0.0, s2.all);
    fast.update(0.25, s2.all);
    check(fast.getBurnTicksLeft(&s2.target) == 2, "one tick done");
    fast.update(0.75, s2.all); // tick + cooldown over -> hit again
    check(fast.getBurnTicksLeft(&s2.target) == 3 && fast.getBurningCount() == 1, "re-hit resets the ticks, still one burn");

    // a critter that leaves the list (dead or escaped) is forgotten
    std::vector<Critter*> none;
    fast.update(0.25, none);
    check(fast.getBurningCount() == 0, "burns of critters no longer in the game are dropped");
}

static void testFreezing() {
    std::cout << "\nFreezingDecorator\n";
    Scene s;
    int coins = 500;
    FreezingDecorator tower(new BasicTower(0, 2));
    check(tower.getSlowFactor() == 0.5 && tower.getSlowDuration() == 2.0, "defaults: factor 0.5, 2 s");
    tower.purchase(coins);
    check(s.target.getSlowTimeLeft() == 0.0, "the target is not slowed before the attack");
    tower.update(0.0, s.all);
    check(s.target.getSlowTimeLeft() > 0.0, "after the attack the target's getSlowTimeLeft() > 0");
    check(s.target.getHitPoints() == 64 && s.target.getCurrentSpeed() < s.target.getSpeed(), "the target took the damage and is slower");

    s.target.update(1.5); // the target walks on, 0.5 s of slow left
    tower.setTargetingStrategy(std::make_unique<WeakestCritterStrategy>()); // keep shooting the same critter
    tower.update(1.0, s.all);
    check(std::fabs(s.target.getSlowTimeLeft() - 2.0) < 1e-9, "a new hit restarts the slow at 2 s (not 2.5 s)");

    FreezingDecorator clamped(new BasicTower(0, 0), 5.0, -1.0);
    check(clamped.getSlowFactor() == 1.0 && clamped.getSlowDuration() == 0.01, "out-of-range values are clamped without exception");
}

static void testStacked() {
    std::cout << "\nStacked decorators in a shot\n";
    Scene s;
    int coins = 500;
    std::unique_ptr<Tower> tower = std::make_unique<BurningDecorator>(
        new FreezingDecorator(new SplashDecorator(new LevelUpDecorator(new BasicTower(0, 2)))));
    tower->purchase(coins);
    tower->update(0.0, s.all);
    check(s.target.getHitPoints() == 84 - 24, "target takes the Lv2 damage (24)");
    check(s.neighbour.getHitPoints() == 84 - 12, "neighbour takes the splash (12, half of 24)");
    check(s.target.getSlowTimeLeft() > 0.0, "target is slowed");
    check(s.far.getHitPoints() == 84 && s.far.getSlowTimeLeft() == 0.0, "far critter untouched");
    tower->update(1.0, s.all);
    check(s.target.getHitPoints() == 84 - 24 - 4 - 24, "next update: one burn tick (4) and a second shot (24)");

    // targeting strategies still work through the decorators
    Scene s2;
    s2.far.takeDamage(80);
    std::unique_ptr<Tower> sniper = std::make_unique<LevelUpDecorator>(new LevelUpDecorator(new BasicTower(0, 4))); // range 4.32
    sniper->setTargetingStrategy(std::make_unique<WeakestCritterStrategy>());
    check(sniper->selectTarget(s2.all) == &s2.far, "a decorated tower uses its strategy with the decorated range");
}

// Level Up as the way towers are upgraded during play (levelUp helper of TowerUpgrades.h)
static void testLevelUpHelper() {
    std::cout << "\nLevel Up through levelUp()\n";
    int coins = 1000;
    std::unique_ptr<Tower> tower = std::make_unique<BasicTower>(1, 1); // cost 100, max level 3
    tower->purchase(coins);                                            // 900 coins left

    const int refundBefore = tower->getRefundValue(); // 50
    check(tower->getLevel() == 1 && tower->getUpgradeCost() == 100, "level 1, next level costs 100 (cost x level)");

    check(levelUp(tower, coins) && coins == 800, "levelUp takes the upgrade cost (100)");
    check(tower->getLevel() == 2 && std::string(tower->getName()) == "Basic Tower Lv2", "level 2, the name says Lv2");
    check(tower->getUpgradeCost() == 200, "the next level costs more (200)");
    check(tower->getRefundValue() == refundBefore + 50, "the refund goes up by half of the price paid (+50)");
    check(std::fabs(tower->getFireRate() - 1.1) < 1e-9, "fire rate x1.1");

    check(levelUp(tower, coins) && coins == 600 && tower->getLevel() == 3, "second levelUp: level 3, 200 taken");
    check(tower->getUpgradeCost() == 0, "upgrade cost is 0 at the maximum level");
    check(!levelUp(tower, coins) && coins == 600 && tower->getLevel() == 3, "levelUp is refused at the maximum level");
    check(tower->getRefundValue() == 50 + 50 + 100, "refund: half of the tower and of both levels (200)");

    // upgrade() on a decorated tower must not add a level a second time
    check(!tower->upgrade(coins) && coins == 600 && tower->getLevel() == 3, "upgrade() on a level-up decorated tower is refused");
    std::unique_ptr<Tower> outer = std::make_unique<SplashDecorator>(new LevelUpDecorator(new BasicTower(0, 0)));
    int moreCoins = 1000;
    outer->purchase(moreCoins);
    check(!outer->upgrade(moreCoins) && outer->getLevel() == 2 && moreCoins == 900,
          "upgrade() is also refused through an outer decorator");

    int sellCoins = 0;
    check(tower->sell(sellCoins) == 200 && sellCoins == 200 && tower->isSold(), "sell pays the decorated refund (200)");
    check(!levelUp(tower, coins), "levelUp is refused on a sold tower");

    int poor = 99;
    std::unique_ptr<Tower> cheap = std::make_unique<BasicTower>(0, 0);
    int startCoins = 100;
    cheap->purchase(startCoins);
    check(!levelUp(cheap, poor) && poor == 99 && cheap->getLevel() == 1 && dynamic_cast<TowerDecorator*>(cheap.get()) == nullptr,
          "levelUp is refused without enough coins (tower and coins unchanged)");

    std::unique_ptr<Tower> notBought = std::make_unique<BasicTower>(0, 0);
    check(!levelUp(notBought, coins) && notBought->getLevel() == 1, "levelUp is refused on a tower that is not purchased");
    std::unique_ptr<Tower> none;
    check(!levelUp(none, coins), "levelUp is refused on an empty pointer");
}

static void testSpecialHelpers() {
    std::cout << "\nSpecial upgrades through the helpers\n";
    int coins = 1000;
    std::unique_ptr<Tower> tower = std::make_unique<BasicTower>(1, 1);
    tower->purchase(coins); // 900

    check(addSplash(tower, coins) && coins == 900 - SPLASH_UPGRADE_COST && hasDecorator<SplashDecorator>(*tower),
          "addSplash wraps the tower and takes its price");
    check(!addSplash(tower, coins) && coins == 900 - SPLASH_UPGRADE_COST, "the same special upgrade cannot be bought twice");
    check(addBurning(tower, coins) && addFreezing(tower, coins), "Burning and Freezing can be added on top");
    check(coins == 900 - SPLASH_UPGRADE_COST - BURNING_UPGRADE_COST - FREEZING_UPGRADE_COST, "each price was taken once");
    check(levelUp(tower, coins) && tower->getLevel() == 2 && std::string(tower->getName()) == "Basic Tower Lv2 +Splash +Burning +Freezing",
          "Level Up still works on top of special upgrades");
    check(dynamic_cast<const BasicTower*>(&baseTower(*tower)) != nullptr, "baseTower finds the Basic Tower under the decorators");

    int poor = FREEZING_UPGRADE_COST - 1;
    std::unique_ptr<Tower> other = std::make_unique<BasicTower>(0, 0);
    int start = 100;
    other->purchase(start);
    check(!canAddFreezing(*other, poor) && !addFreezing(other, poor) && poor == FREEZING_UPGRADE_COST - 1,
          "a special upgrade is refused without enough coins");
    std::unique_ptr<Tower> notBought = std::make_unique<BasicTower>(0, 0);
    check(!addBurning(notBought, coins), "a special upgrade is refused on a tower that is not purchased");
}

static void testStrategySurvivesWrapping() {
    std::cout << "\nTargeting strategy and decorators\n";
    int coins = 1000;
    std::unique_ptr<Tower> tower = std::make_unique<BasicTower>(0, 4);
    tower->purchase(coins);
    tower->setTargetingStrategy(std::make_unique<WeakestCritterStrategy>());

    levelUp(tower, coins);
    addBurning(tower, coins);
    check(std::string(tower->getTargetingStrategy().getName()) == "Weakest", "the strategy set before the upgrades is kept");

    Scene s;
    s.far.takeDamage(80);
    check(tower->selectTarget(s.all) == &s.far, "the upgraded tower still shoots the weakest critter in range");

    tower->setTargetingStrategy(nullptr);
    check(std::string(tower->getTargetingStrategy().getName()) == "Nearest", "a decorated tower can change its strategy");
}

int main() {
    std::cout << "=== Tower decorator tests ===\n\n";
    testLevelUp();
    testForwarding();
    testRange();
    testNames();
    testSplash();
    testBurning();
    testFreezing();
    testStacked();
    testLevelUpHelper();
    testSpecialHelpers();
    testStrategySurvivesWrapping();
    return testSummary();
}

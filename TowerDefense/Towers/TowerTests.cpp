#include "Tower.h"
#include "Critter.h"
#include "Map.h"
#include "TestHelper.h"

#include <vector>

// straight map: the critters walk along the top row from (0, 0) to (0, 9), the towers stand below it.
static Map makeStraightMap() {
    Map map(3, 10);
    map.setEntry(0, 0);
    map.setExit(0, 9);
    for (int column = 1; column < 9; column++) map.setCell(0, column, CellType::PATH);
    return map;
}

// puts a critter of level 1 on the entry and walks it to the given column.
static Critter placeCritter(const Map& map, CritterType type, int column) {
    Critter critter(type, 1);
    critter.spawn(map);
    critter.update(static_cast<double>(column) / critter.getSpeed());
    return critter;
}

int main() {
    std::cout << "Part 2 tests (towers)\n\n";

    std::cout << "Buying, upgrading and selling\n";
    int coins = 1000;
    BasicTower basic(1, 1);
    check(basic.getCost() == 100 && basic.getRefundValue() == 50, "a basic tower costs 100 and refunds half of it");
    check(!basic.upgrade(coins) && !basic.update(1.0, {}), "a tower that is not bought cannot be upgraded or shoot");
    check(basic.purchase(coins) && coins == 900 && basic.isPurchased(), "buying takes the cost from the coins");
    check(!basic.purchase(coins) && coins == 900, "a tower cannot be bought twice");

    int poor = 50;
    BasicTower unaffordable(1, 2);
    check(!unaffordable.purchase(poor) && poor == 50, "a tower cannot be bought without enough coins");

    int damage1 = basic.getDamage();
    double range1 = basic.getRange();
    double rate1 = basic.getFireRate();
    check(basic.upgrade(coins) && basic.getLevel() == 2 && coins == 800, "upgrading costs coins and gives level 2");
    check(basic.getDamage() > damage1 && basic.getRange() > range1 && basic.getFireRate() > rate1, "a higher level has more damage, range and fire rate");
    check(basic.upgrade(coins) && basic.getLevel() == 3, "a tower can reach level 3");
    check(!basic.upgrade(coins) && basic.getLevel() == 3, "a tower cannot go over its maximum level");
    int refund = basic.getRefundValue();
    check(refund == (100 + 100 + 200) / 2, "the refund depends on the coins invested in the tower");
    int before = coins;
    check(basic.sell(coins) == refund && coins == before + refund && basic.isSold(), "selling gives the refund");
    check(basic.sell(coins) == 0 && !basic.upgrade(coins), "a sold tower cannot be sold or upgraded again");

    std::cout << "Range and target selection\n";
    Map map = makeStraightMap();
    coins = 1000;
    BasicTower tower(1, 5); // range 3
    tower.purchase(coins);
    check(tower.isInRange(0, 5) && tower.isInRange(0, 7) && !tower.isInRange(0, 9), "range is measured from the tower in cells");

    Critter far = placeCritter(map, CritterType::MELEE, 8);
    Critter near = placeCritter(map, CritterType::MELEE, 4);
    std::vector<Critter*> critters = {&far, &near};
    int farHp = far.getHitPoints();
    int nearHp = near.getHitPoints();
    check(tower.update(0.0, critters), "a tower shoots when there is a critter in range");
    check(far.getHitPoints() == farHp && near.getHitPoints() == nearHp - tower.getDamage(), "it picks the closest critter in range and ignores the one out of range");
    check(!tower.update(0.0, critters), "it cannot shoot again before its cooldown is over");
    check(!tower.update(0.5, critters) && tower.update(0.5, critters), "it shoots again after 1 / fire rate seconds");

    std::vector<Critter*> noneInRange = {&far};
    BasicTower lonely(2, 0);
    lonely.purchase(coins);
    check(!lonely.update(1.0, noneInRange) && far.getHitPoints() == farHp, "nothing happens when no critter is in range");

    std::cout << "Tower types\n";
    BasicTower direct(1, 5);
    SplashTower splash(1, 5);
    SlowTower slow(1, 5);
    direct.purchase(coins);
    splash.purchase(coins);
    slow.purchase(coins);

    Critter a = placeCritter(map, CritterType::MELEE, 4);
    Critter b = placeCritter(map, CritterType::MELEE, 5);
    Critter c = placeCritter(map, CritterType::MELEE, 8);
    std::vector<Critter*> group = {&a, &b, &c};
    int hpA = a.getHitPoints();
    int hpB = b.getHitPoints();
    int hpC = c.getHitPoints();
    splash.update(0.0, group);
    check(a.getHitPoints() < hpA && b.getHitPoints() < hpB, "the splash tower hurts every critter close to its target");
    check(c.getHitPoints() == hpC, "the splash tower does not hurt critters outside the splash radius");

    Critter d = placeCritter(map, CritterType::MELEE, 4);
    std::vector<Critter*> single = {&d};
    double normalSpeed = d.getCurrentSpeed();
    slow.update(0.0, single);
    check(d.getCurrentSpeed() < normalSpeed && d.getHitPoints() < d.getMaxHitPoints(), "the slow tower damages the critter and makes it slower");

    Critter e = placeCritter(map, CritterType::MELEE, 4);
    std::vector<Critter*> alone = {&e};
    direct.update(0.0, alone);
    check(e.getCurrentSpeed() == normalSpeed, "the basic tower only does direct damage");

    std::cout << "Kills\n";
    Critter weak = placeCritter(map, CritterType::CASTER, 4);
    std::vector<Critter*> weakList = {&weak};
    BasicTower killer(1, 4);
    killer.purchase(coins);
    for (int i = 0; i < 5 && weak.isAlive(); i++) killer.update(1.0, weakList);
    check(!weak.isAlive() && weak.getState() == CritterState::DEAD, "enough shots kill a critter");
    check(!killer.update(1.0, weakList), "a tower does not shoot dead critters");

    return testSummary();
}

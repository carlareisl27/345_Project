#include "Map.h"
#include "Critter.h"
#include "CritterGroup.h"
#include "CritterGroupGenerator.h"
#include "TestHelper.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

// same 5x5 map used by the Map driver: the shortest path has 8 cells (7 steps).
static Map makeSimpleMap() {
    Map map(5, 5);
    map.setEntry(0, 0);
    map.setExit(3, 4);
    map.setCell(0, 1, CellType::PATH);
    map.setCell(0, 2, CellType::PATH);
    map.setCell(1, 2, CellType::PATH);
    map.setCell(2, 2, CellType::PATH);
    map.setCell(2, 3, CellType::PATH);
    map.setCell(2, 4, CellType::PATH);
    map.setCell(3, 4, CellType::PATH);
    return map;
}

// map with a dead end: the path goes along the top and down the right side, the left column is a dead end.
static Map makeDeadEndMap() {
    Map map(5, 5);
    map.setEntry(0, 0);
    map.setExit(4, 4);
    for (int column = 1; column < 5; column++) {
        map.setCell(0, column, CellType::PATH);
    }
    for (int row = 1; row < 4; row++) {
        map.setCell(row, 4, CellType::PATH);
        map.setCell(row, 0, CellType::PATH); // dead end, it never reaches the exit.
    }
    return map;
}

// 1. stats of a single critter.
static void testCritterStats() {
    std::cout << "Critter stats\n";

    const CritterType types[] = {CritterType::MELEE, CritterType::CASTER, CritterType::CANNON, CritterType::SUPER};

    bool rewardProportional = true;
    bool statsGrow = true;
    for (CritterType type : types) {
        Critter level1(type, 1);
        for (int level = 2; level <= 30; level++) {
            Critter higher(type, level);
            Critter lower(type, level - 1);
            if (higher.getReward() != level1.getReward() * level) rewardProportional = false;
            if (higher.getMaxHitPoints() <= lower.getMaxHitPoints()) statsGrow = false;
            if (higher.getStrength() < lower.getStrength()) statsGrow = false;
            if (higher.getSpeed() < lower.getSpeed()) statsGrow = false;
        }
    }

    check(rewardProportional, "reward is proportional to the level for every type");
    check(statsGrow, "hit points grow every level, strength and speed never go down");
    check(Critter(CritterType::MELEE, 0).getLevel() == 1, "a level below 1 becomes level 1");
    check(Critter(CritterType::CANNON, 1).getMaxHitPoints() > Critter(CritterType::CASTER, 1).getMaxHitPoints(),
          "cannon has more hit points than caster");
}

// 2. taking damage and dying.
static void testDamage() {
    std::cout << "Damage and death\n";
    Map map = makeSimpleMap();

    Critter waiting(CritterType::MELEE, 1);
    check(!waiting.takeDamage(10) && waiting.getHitPoints() == waiting.getMaxHitPoints(),
          "a critter that has not spawned cannot be damaged");

    Critter critter(CritterType::MELEE, 1);
    critter.spawn(map);
    int before = critter.getHitPoints();
    check(!critter.takeDamage(5) && critter.getHitPoints() == before - 5, "damage reduces hit points");
    check(!critter.takeDamage(0) && !critter.takeDamage(-3) && critter.getHitPoints() == before - 5,
          "zero or negative damage does nothing");
    check(critter.takeDamage(100000), "the hit that brings hit points to zero kills the critter");
    check(critter.getHitPoints() == 0 && !critter.isAlive() && critter.getState() == CritterState::DEAD,
          "hit points never go below zero and the critter is dead");
    check(!critter.takeDamage(5), "a dead critter cannot be killed again");

    int row = critter.getRow();
    critter.update(10.0);
    check(critter.getRow() == row, "a dead critter does not move");
}

// 3. movement along the path.
static void testMovement() {
    std::cout << "Movement\n";
    Map map = makeSimpleMap();

    Critter critter(CritterType::MELEE, 1); // speed is exactly 1 cell per second at level 1.
    check(critter.getRow() == -1 && critter.getColumn() == -1, "before spawning the critter is not on the map");
    check(critter.spawn(map), "spawn works on a valid map");
    check(!critter.spawn(map), "a critter cannot spawn twice");
    check(critter.getRow() == 0 && critter.getColumn() == 0, "the critter starts on the entry");
    check(critter.getPathLength() == 8, "the path found has 8 cells");
    check(critter.chooseNextCell() == std::make_pair(0, 1), "first decision is to move to (0, 1)");

    critter.update(1.0);
    check(critter.getRow() == 0 && critter.getColumn() == 1, "after 1 second it is on (0, 1)");
    critter.update(0.5);
    check(critter.getRow() == 0 && critter.getColumn() == 1, "half a step does not change the cell");
    critter.update(0.5);
    check(critter.getRow() == 0 && critter.getColumn() == 2, "two half steps make one step");

    for (int i = 0; i < 9; i++) {
        critter.update(0.5);
    }
    check(!critter.hasEscaped(), "after 6.5 seconds it has not reached the exit");
    critter.update(0.5);
    check(critter.hasEscaped() && critter.getRow() == 3 && critter.getColumn() == 4, "after 7 seconds it reached the exit");

    // the critter always walks on PATH cells, one cell at a time, and avoids the dead end.
    Map deadEnd = makeDeadEndMap();
    Critter walker(CritterType::MELEE, 1);
    walker.spawn(deadEnd);
    bool onlyPathCells = true;
    bool oneStepAtATime = true;
    bool avoidsDeadEnd = true;
    int lastRow = walker.getRow();
    int lastColumn = walker.getColumn();
    for (int i = 0; i < 100 && !walker.hasEscaped(); i++) {
        walker.update(0.25);
        int row = walker.getRow();
        int column = walker.getColumn();
        if (!deadEnd.isPathCell(row, column)) onlyPathCells = false;
        if (std::abs(row - lastRow) + std::abs(column - lastColumn) > 1) oneStepAtATime = false;
        if (column == 0 && row > 0) avoidsDeadEnd = false;
        lastRow = row;
        lastColumn = column;
    }
    check(walker.hasEscaped() && walker.getPathLength() == 9, "it finds the shortest path (9 cells) on the dead end map");
    check(onlyPathCells && oneStepAtATime && avoidsDeadEnd, "it only moves through PATH cells, one cell at a time, and skips the dead end");

    // a map without a connection to the exit.
    Map broken(3, 3);
    broken.setEntry(0, 0);
    broken.setExit(2, 2);
    Critter stuck(CritterType::MELEE, 1);
    check(!stuck.spawn(broken) && stuck.getState() == CritterState::WAITING, "it cannot spawn if there is no path to the exit");
}

// 4. slow effect.
static void testSlow() {
    std::cout << "Slow effect\n";
    Map map = makeSimpleMap();

    Critter critter(CritterType::MELEE, 1);
    critter.spawn(map);
    critter.applySlow(0.5, 100.0);
    check(critter.getCurrentSpeed() == 0.5 * critter.getSpeed(), "slow halves the current speed");
    critter.update(4.0);
    check(critter.getRow() == 0 && critter.getColumn() == 2, "a slowed critter walks 2 cells in 4 seconds");

    Critter shortSlow(CritterType::MELEE, 1);
    shortSlow.spawn(map);
    shortSlow.applySlow(0.5, 1.0);
    shortSlow.update(1.0);
    check(shortSlow.getCurrentSpeed() == shortSlow.getSpeed(), "the slow ends when its duration is over");

    Critter invalid(CritterType::MELEE, 1);
    invalid.spawn(map);
    invalid.applySlow(1.5, 5.0); // a factor above 1 would speed it up, so it is ignored.
    check(invalid.getCurrentSpeed() == invalid.getSpeed(), "a slow factor that would speed the critter up is ignored");
}

// 5. generator: composition and difficulty.
static void testGenerator() {
    std::cout << "Group generator\n";
    Map map = makeSimpleMap();
    CritterGroupGenerator generator(1.0);

    CritterGroup wave1 = generator.generate(1, map);
    CritterGroup wave3 = generator.generate(3, map);
    CritterGroup wave10 = generator.generate(10, map);

    check(wave1.getSize() == 6 && wave1.getCritters()[0].getType() == CritterType::MELEE, "wave 1 has 3 melee and 3 caster critters");
    check(wave3.getSize() == 7 && wave3.getCritters()[3].getType() == CritterType::CANNON, "wave 3 adds a cannon after the melee critters");
    check(wave10.getSize() == 10 && wave10.getCritters()[0].getType() == CritterType::SUPER, "wave 10 has a super critter in front and 3 cannons (10 critters)");
    check(wave3.getCritters()[0].getLevel() == 3, "the level of the critters is the number of the wave");

    bool harder = true;
    bool neverSmaller = true;
    for (int wave = 2; wave <= 30; wave++) {
        CritterGroup previous = generator.generate(wave - 1, map);
        CritterGroup current = generator.generate(wave, map);
        if (current.getTotalHitPoints() <= previous.getTotalHitPoints()) harder = false;
        if (current.getSize() < previous.getSize()) neverSmaller = false;
    }
    check(harder, "total hit points increase in every wave from 1 to 30");
    check(neverSmaller, "a wave never has fewer critters than the previous one");

    check(generator.getSpawnInterval(1) == 1.0 && wave3.getSpawnInterval() < wave1.getSpawnInterval(),
          "wave 1 uses the base spawn interval and wave 3 releases critters faster");
    bool tighter = true;
    for (int wave = 2; wave <= 40; wave++) {
        if (generator.getSpawnInterval(wave) > generator.getSpawnInterval(wave - 1)) tighter = false;
    }
    check(tighter && generator.getSpawnInterval(40) == 0.4, "the interval never grows and stops at 40% of the base interval");

    bool threwForWave = false;
    try { generator.generate(0, map); } catch (const std::invalid_argument&) { threwForWave = true; }
    check(threwForWave, "wave 0 is rejected");

    Map noExit(4, 4);
    noExit.setEntry(0, 0);
    bool threwForMap = false;
    try { generator.generate(1, noExit); } catch (const std::invalid_argument&) { threwForMap = true; }
    check(threwForMap, "an invalid map is rejected");

    bool threwForInterval = false;
    try { CritterGroupGenerator bad(0.0); } catch (const std::invalid_argument&) { threwForInterval = true; }
    check(threwForInterval, "a spawn interval of 0 is rejected");
}

// advances the wave in small steps (0.25 s): a critter cannot be released onto an occupied entry,
// so one huge step cannot release several critters at once.
static void advance(CritterGroup& group, const Map& map, int& coins, double seconds) {
    for (double done = 0.0; done < seconds - 1e-9; done += 0.25) {
        group.update(0.25, map, coins);
    }
}

// 6. critters appear one after the other.
static void testSequentialSpawn() {
    std::cout << "Sequential spawning\n";
    Map map = makeSimpleMap();
    CritterGroupGenerator generator(1.0);
    CritterGroup group = generator.generate(1, map);
    int coins = 100;

    check(group.getSpawnedCount() == 0 && group.getActiveCritters().empty(), "nothing is on the map before the wave starts");
    group.update(0.5, map, coins);
    check(group.getSpawnedCount() == 1, "the first critter appears right away");
    group.update(0.25, map, coins);
    check(group.getSpawnedCount() == 1, "the second critter is not there before the interval ends");
    group.update(0.25, map, coins);
    check(group.getSpawnedCount() == 2, "the second critter appears after one interval");
    advance(group, map, coins, 3.0);
    check(group.getSpawnedCount() == 5, "one critter appears every interval");
    advance(group, map, coins, 1.0);
    check(group.getSpawnedCount() == 6, "all 6 critters of wave 1 have appeared");

    std::vector<Critter*> active = group.getActiveCritters();
    bool allOnMap = true;
    for (Critter* critter : active) {
        if (!map.isPathCell(critter->getRow(), critter->getColumn())) allOnMap = false;
    }
    check(allOnMap && !active.empty(), "active critters are on PATH cells");
}

// 7. rewards when killed and coins stolen when they escape.
static void testCoins() {
    std::cout << "Rewards and stolen coins\n";
    Map map = makeSimpleMap();
    CritterGroupGenerator generator(1.0);

    // a killed critter gives its reward once.
    CritterGroup group = generator.generate(2, map);
    int coins = 50;
    group.update(0.1, map, coins);
    Critter* first = group.getActiveCritters()[0];
    int reward = first->getReward();
    first->takeDamage(100000);
    group.update(0.1, map, coins);
    check(coins == 50 + reward && group.getKilledCount() == 1, "killing a critter gives its reward (level 2 melee = 4 coins)");
    group.update(0.1, map, coins);
    check(coins == 50 + reward, "the reward is given only once");

    // nobody attacks: every critter reaches the exit and steals strength coins.
    CritterGroup undefended = generator.generate(1, map);
    coins = 100;
    for (int i = 0; i < 400 && !undefended.isFinished(); i++) {
        undefended.update(0.1, map, coins);
    }
    check(undefended.isFinished() && undefended.getEscapedCount() == 6, "all 6 critters of wave 1 escape if nobody attacks them");
    check(undefended.getCoinsStolen() == 9 && coins == 91, "they steal 9 coins (3 melee x 1 + 3 casters x 2)");

    // the player can never have less than 0 coins.
    CritterGroup poor = generator.generate(1, map);
    coins = 4;
    for (int i = 0; i < 400 && !poor.isFinished(); i++) {
        poor.update(0.1, map, coins);
    }
    check(coins == 0 && poor.getCoinsStolen() == 4, "the coins of the player never go below 0");

    // all critters are killed as soon as they appear.
    CritterGroup defended = generator.generate(3, map);
    coins = 0;
    int expected = 0;
    for (const Critter& critter : defended.getCritters()) {
        expected += critter.getReward();
    }
    for (int i = 0; i < 400 && !defended.isFinished(); i++) {
        defended.update(0.1, map, coins);
        for (Critter* critter : defended.getActiveCritters()) {
            critter->takeDamage(100000);
        }
    }
    defended.update(0.1, map, coins); // collects the last reward.
    check(defended.isFinished() && defended.getKilledCount() == defended.getSize() && defended.getEscapedCount() == 0,
          "killing every critter finishes the wave without escapes");
    check(coins == expected && defended.getCoinsEarned() == expected, "the player receives the reward of every killed critter");
}

// 8. one game element per cell: critters never share a path cell.
static bool sharesACell(CritterGroup& group) {
    std::vector<Critter*> active = group.getActiveCritters();
    for (size_t i = 0; i < active.size(); i++) {
        for (size_t j = i + 1; j < active.size(); j++) {
            if (active[i]->getRow() == active[j]->getRow() && active[i]->getColumn() == active[j]->getColumn()) {
                return true;
            }
        }
    }
    return false;
}

static void testOneCritterPerCell() {
    std::cout << "One critter per cell\n";
    Map map = makeSimpleMap();

    Critter blocked(CritterType::MELEE, 1);
    blocked.spawn(map);
    bool taken = true;
    auto isOccupied = [&taken](int row, int column) { return taken && row == 0 && column == 1; };
    blocked.update(5.0, isOccupied);
    check(blocked.getRow() == 0 && blocked.getColumn() == 0, "a critter waits while the next cell is occupied");
    taken = false;
    blocked.update(1.0, isOccupied);
    check(blocked.getRow() == 0 && blocked.getColumn() == 1, "it moves on as soon as the cell is free");

    // a fast critter released right behind a slow one: it must queue behind it, never share its cell.
    std::vector<Critter> critters = {
        Critter(CritterType::CANNON, 1), // 0.8 cells per second
        Critter(CritterType::CASTER, 1), // 1.1 cells per second
        Critter(CritterType::CASTER, 1),
    };
    CritterGroup group(1, critters, 0.1);
    int coins = 100;
    bool everShared = false;
    for (int i = 0; i < 2000 && !group.isFinished(); i++) {
        group.update(0.05, map, coins);
        everShared = everShared || sharesACell(group);
    }
    check(!everShared, "two critters are never on the same cell");
    check(group.isFinished() && group.getEscapedCount() == 3, "queued critters still reach the exit");

    // the entry is occupied while the first critter is on it: the second critter waits to spawn.
    CritterGroup tight(1, {Critter(CritterType::MELEE, 1), Critter(CritterType::MELEE, 1)}, 0.1);
    coins = 100;
    tight.update(0.0, map, coins);
    tight.update(0.2, map, coins);
    check(tight.getSpawnedCount() == 1, "the next critter does not spawn on an occupied entry");
    tight.update(1.0, map, coins);
    tight.update(0.1, map, coins);
    check(tight.getSpawnedCount() == 2, "it spawns once the entry is free");
}

int main() {
    std::cout << "Part 3 tests (critters and critter group generator)\n\n";

    testCritterStats();
    testDamage();
    testMovement();
    testSlow();
    testGenerator();
    testSequentialSpawn();
    testCoins();
    testOneCritterPerCell();

    return testSummary();
}

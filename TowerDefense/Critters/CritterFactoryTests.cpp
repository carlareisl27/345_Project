#include "Map.h"
#include "Critter.h"
#include "CritterGroup.h"
#include "CritterGroupGenerator.h"
#include "CritterFactory.h"
#include "StandardCritterFactory.h"
#include "TestHelper.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

// Tests of Assignment 3, Part 2 (Factory pattern): StandardCritterFactory used through the CritterFactory interface.

// same 5x5 map used in CritterTests.cpp
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

static int countType(const CritterGroup& group, CritterType type) {
    const auto& critters = group.getCritters();
    return static_cast<int>(std::count_if(critters.begin(), critters.end(), [type](const Critter& c) { return c.getType() == type; }));
}

static bool allLevel(const CritterGroup& group, int level) {
    for (const Critter& critter : group.getCritters()) {
        if (critter.getLevel() != level) return false;
    }
    return true;
}

static void testComposition(const CritterFactory& factory, const Map& map) {
    std::cout << "Wave composition\n";

    CritterGroup wave1 = factory.createGroup(1, map);
    check(countType(wave1, CritterType::MELEE) == 3 && countType(wave1, CritterType::CASTER) == 3
              && countType(wave1, CritterType::CANNON) == 0 && countType(wave1, CritterType::SUPER) == 0 && wave1.getSize() == 6,
          "wave 1 has exactly 3 MELEE and 3 CASTER critters and nothing else");

    check(countType(factory.createGroup(2, map), CritterType::CANNON) == 0, "wave 2 has no CANNON critter yet");
    check(countType(factory.createGroup(3, map), CritterType::CANNON) >= 1, "wave 3 has at least one CANNON critter");
    check(countType(factory.createGroup(9, map), CritterType::SUPER) == 0, "wave 9 has no SUPER critter yet");
    check(countType(factory.createGroup(10, map), CritterType::SUPER) >= 1, "wave 10 has at least one SUPER critter");

    bool formula = true;
    bool neverEasier = true;
    int previousSize = 0;
    for (int wave = 1; wave <= 30; wave++) {
        CritterGroup group = factory.createGroup(wave, map);
        if (group.getSize() != 6 + wave / 3 + wave / 10) formula = false;
        if (countType(group, CritterType::CANNON) != wave / 3 || countType(group, CritterType::SUPER) != wave / 10) formula = false;
        if (group.getWaveNumber() != wave) formula = false;
        if (group.getSize() < previousSize) neverEasier = false;
        previousSize = group.getSize();
    }
    check(formula, "waves 1 to 30: size 6 + N/3 + N/10 with N/3 CANNON and N/10 SUPER, wave number kept");
    check(neverEasier, "waves 1 to 30: a wave never has fewer critters than the previous one");
}

static void testLevels(const CritterFactory& factory, const Map& map) {
    std::cout << "\nCritter levels\n";
    check(allLevel(factory.createGroup(1, map), 1), "every critter of wave 1 has level 1");
    check(allLevel(factory.createGroup(5, map), 5), "every critter of wave 5 has level 5");
    check(allLevel(factory.createGroup(10, map), 10), "every critter of wave 10 has level 10");
    check(factory.createGroup(10, map).getTotalHitPoints() > factory.createGroup(5, map).getTotalHitPoints(),
          "wave 10 has more total hit points than wave 5");
}

static void testSpawnInterval(const Map& map) {
    std::cout << "\nSpawn interval\n";
    StandardCritterFactory factory(1.0);
    bool formula = true;
    for (int wave = 1; wave <= 30; wave++) {
        const double expected = 1.0 * std::max(1.0 - 0.04 * (wave - 1), 0.4);
        if (std::fabs(factory.createGroup(wave, map).getSpawnInterval() - expected) > 1e-9) formula = false;
    }
    check(formula, "interval = base x max(1 - 0.04 (N - 1), 0.4) for waves 1 to 30");
    check(std::fabs(StandardCritterFactory().createGroup(1, map).getSpawnInterval() - 0.8) < 1e-9, "default base interval is 0.8 s");
    bool thrown = false;
    try { StandardCritterFactory bad(0.0); } catch (const std::invalid_argument&) { thrown = true; }
    check(thrown, "a base interval of 0 is refused");
}

static void testSameAsGenerator(const CritterFactory& factory, const Map& map) {
    std::cout << "\nSame waves as CritterGroupGenerator\n";
    CritterGroupGenerator generator;
    bool same = true;
    for (int wave = 1; wave <= 30; wave++) {
        CritterGroup fromFactory = factory.createGroup(wave, map);
        CritterGroup fromGenerator = generator.generate(wave, map);
        if (fromFactory.getSize() != fromGenerator.getSize() || fromFactory.getSpawnInterval() != fromGenerator.getSpawnInterval()) {
            same = false;
            continue;
        }
        for (int i = 0; i < fromFactory.getSize(); i++) {
            const Critter& a = fromFactory.getCritters()[i];
            const Critter& b = fromGenerator.getCritters()[i];
            if (a.getType() != b.getType() || a.getLevel() != b.getLevel()) same = false;
        }
    }
    check(same, "count, types, order, levels and interval identical for waves 1 to 30");
}

static void testInvalidRequests(const CritterFactory& factory, const Map& map) {
    std::cout << "\nInvalid requests\n";
    for (int wave : {0, -1, -10}) {
        bool thrown = false;
        try {
            factory.createGroup(wave, map);
        } catch (const std::invalid_argument& error) {
            thrown = std::string(error.what()).size() > 0;
        }
        check(thrown, wave == 0 ? "wave 0 throws std::invalid_argument" : "a negative wave throws std::invalid_argument");
    }

    Map unfinished(5, 5);
    bool thrown = false;
    try {
        factory.createGroup(1, unfinished);
    } catch (const std::invalid_argument& error) {
        thrown = std::string(error.what()).size() > 0;
    }
    check(thrown, "an invalid map throws std::invalid_argument");
}

int main() {
    std::cout << "=== Critter factory tests ===\n\n";
    Map map = makeSimpleMap();
    StandardCritterFactory standard;
    const CritterFactory& factory = standard; // every test goes through the interface

    testComposition(factory, map);
    testLevels(factory, map);
    testSpawnInterval(map);
    testSameAsGenerator(factory, map);
    testInvalidRequests(factory, map);
    return testSummary();
}

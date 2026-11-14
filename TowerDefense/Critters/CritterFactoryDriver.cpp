#include "Map.h"
#include "Critter.h"
#include "CritterGroup.h"
#include "CritterFactory.h"
#include "StandardCritterFactory.h"

#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>

// Driver for Assignment 3, Part 2 (Factory pattern).
// The game asks a CritterFactory for wave N and receives a ready CritterGroup. The further the game goes, the more
// critters, the stronger types and the higher levels the factory puts in the wave, and the faster they are released.

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

// the game only knows the CritterFactory interface
static void describeWave(const CritterFactory& factory, int waveNumber, const Map& map) {
    CritterGroup group = factory.createGroup(waveNumber, map);

    std::map<CritterType, int> counts;
    for (const Critter& critter : group.getCritters()) {
        counts[critter.getType()]++;
    }

    std::cout << "Wave " << group.getWaveNumber() << ": " << group.getSize() << " critters, level "
              << group.getCritters().front().getLevel() << ", one every " << std::fixed << std::setprecision(3)
              << group.getSpawnInterval() << " s, total hit points " << group.getTotalHitPoints() << '\n';

    for (const auto& entry : counts) {
        Critter sample(entry.first, waveNumber);
        std::cout << "  " << std::left << std::setw(7) << sample.getName() << std::right << " x" << entry.second
                  << "  (" << sample.getMaxHitPoints() << " HP, reward " << sample.getReward() << " each)\n";
    }

    std::cout << "  walking order: ";
    for (const Critter& critter : group.getCritters()) {
        std::cout << critter.getSymbol();
    }
    std::cout << "\n\n";
}

int main() {
    std::cout << "=== Part 2: Critter waves (Factory pattern) ===\n\n";

    Map map = makeSimpleMap();
    StandardCritterFactory standard;
    const CritterFactory& factory = standard;

    for (int wave : {1, 5, 10}) {
        describeWave(factory, wave, map);
    }

    std::cout << "Difficulty curve (critter count = 6 + N/3 + N/10):\n";
    std::cout << "  wave:     ";
    for (int wave = 1; wave <= 30; wave += 3) std::cout << std::setw(5) << wave;
    std::cout << "\n  critters: ";
    for (int wave = 1; wave <= 30; wave += 3) std::cout << std::setw(5) << factory.createGroup(wave, map).getSize();
    std::cout << "\n  total HP: ";
    for (int wave = 1; wave <= 30; wave += 3) std::cout << std::setw(5) << factory.createGroup(wave, map).getTotalHitPoints();
    std::cout << "\n\n";

    std::cout << "Invalid requests are refused:\n";
    try {
        factory.createGroup(0, map);
    } catch (const std::invalid_argument& error) {
        std::cout << "  wave 0: " << error.what() << '\n';
    }
    try {
        Map unfinished(5, 5); // no entry, no exit
        factory.createGroup(1, unfinished);
    } catch (const std::invalid_argument& error) {
        std::cout << "  invalid map: " << error.what() << '\n';
    }

    return 0;
}

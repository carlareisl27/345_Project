#include "Map.h"
#include "Critter.h"
#include "CritterGroup.h"
#include "CritterGroupGenerator.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>

// Part 3 driver: shows the waves made by the generator and how critters spawn, walk, die and escape.
// No towers are used here (see GameDriver for the full game): the "defense" is a scripted kill.

void drawMap(const Map& map, const CritterGroup& group) {
    for (int row = 0; row < map.getRows(); row++) {
        for (int column = 0; column < map.getColumns(); column++) {
            char symbol = '#';
            if (map.getEntry() == std::make_pair(row, column)) symbol = 'E';
            else if (map.getExit() == std::make_pair(row, column)) symbol = 'X';
            else if (map.isPathCell(row, column)) symbol = '.';

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

void printWave(const CritterGroup& group) {
    std::cout << "Wave " << group.getWaveNumber() << ": " << group.getSize() << " critters, total hit points " << group.getTotalHitPoints()
              << ", one critter every " << std::fixed << std::setprecision(2) << group.getSpawnInterval() << " s\n";
    std::cout << "  " << std::left << std::setw(8) << "type" << std::setw(7) << "level" << std::setw(6) << "HP"
              << std::setw(10) << "strength" << std::setw(7) << "speed" << "reward\n";

    for (const Critter& critter : group.getCritters()) {
        std::cout << "  " << std::left << std::setw(8) << critter.getName() << std::setw(7) << critter.getLevel()
                  << std::setw(6) << critter.getMaxHitPoints() << std::setw(10) << critter.getStrength()
                  << std::setw(7) << critter.getSpeed() << critter.getReward() << '\n';
    }
    std::cout << '\n';
}

// plays a wave; if killAtColumn >= 0 every critter is killed as soon as it walks past that column.
void playWave(const Map& map, CritterGroup& group, int& coins, int killAtColumn) {
    const double deltaTime = 0.1;
    double time = 0.0;
    double nextFrame = 4.0;

    std::cout << "--- Wave " << group.getWaveNumber() << " starts with " << coins << " coins ---\n";
    std::cout << std::right;

    while (!group.isFinished() && time < 120.0) {
        int killedBefore = group.getKilledCount();
        int escapedBefore = group.getEscapedCount();
        int spawnedBefore = group.getSpawnedCount();
        int coinsBefore = coins;

        if (killAtColumn >= 0) {
            for (Critter* critter : group.getActiveCritters()) {
                if (critter->getRow() == 2 && critter->getColumn() <= killAtColumn) critter->takeDamage(100000);
            }
        }
        group.update(deltaTime, map, coins);
        time += deltaTime;

        std::cout << std::fixed << std::setprecision(1);
        if (group.getSpawnedCount() > spawnedBefore) {
            const Critter& spawned = group.getCritters()[group.getSpawnedCount() - 1];
            std::cout << "t=" << std::setw(4) << time << "s  " << spawned.getName() << " (level " << spawned.getLevel() << ", "
                      << spawned.getMaxHitPoints() << " HP) enters the map\n";
        }
        if (group.getKilledCount() > killedBefore) {
            std::cout << "t=" << std::setw(4) << time << "s  critter killed, +" << coins - coinsBefore << " coins -> " << coins << '\n';
        }
        if (group.getEscapedCount() > escapedBefore) {
            std::cout << "t=" << std::setw(4) << time << "s  critter reached the exit, -" << coinsBefore - coins << " coins -> " << coins << '\n';
        }
        if (time >= nextFrame && nextFrame < 9.0) {
            std::cout << "\nMap at t=" << time << "s:\n";
            drawMap(map, group);
            std::cout << '\n';
            nextFrame += 5.0;
        }
    }

    std::cout << "Wave " << group.getWaveNumber() << " finished: " << group.getKilledCount() << " killed (+" << group.getCoinsEarned()
              << " coins), " << group.getEscapedCount() << " escaped (-" << group.getCoinsStolen() << " coins). Coins now: " << coins << "\n\n";
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

    std::cout << "=== Part 3 driver: critter group generator ===\n\nMap (E entry, X exit, . path, # scenery):\n";
    map.displayMap();
    std::cout << (map.isValidMap() ? "The map is valid.\n\n" : "The map is invalid.\n\n");

    CritterGroupGenerator generator;

    std::cout << "=== 1. Groups created by the generator ===\n";
    for (int wave : {1, 3, 10}) {
        printWave(generator.generate(wave, map));
    }

    std::cout << "=== 2. Wave 1 with no defense: every critter reaches the exit and steals coins ===\n";
    int coins = 50;
    CritterGroup undefended = generator.generate(1, map);
    playWave(map, undefended, coins, -1);

    std::cout << "=== 3. Wave 2 where every critter is killed on the second row: rewards ===\n";
    CritterGroup defended = generator.generate(2, map);
    playWave(map, defended, coins, 5);

    std::cout << "=== 4. Invalid map ===\n";
    Map invalidMap(4, 4);
    invalidMap.setEntry(0, 0);
    try {
        generator.generate(1, invalidMap);
    } catch (const std::invalid_argument& error) {
        std::cout << "Generator refused the map: " << error.what() << '\n';
    }

    return 0;
}

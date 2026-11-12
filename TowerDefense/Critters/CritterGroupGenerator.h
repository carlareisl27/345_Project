#ifndef CRITTERGROUPGENERATOR_H
#define CRITTERGROUPGENERATOR_H

#include "CritterGroup.h"

class Map;

/// Creates the group of critters of each wave. Critter level = wave number, so every wave is harder than the previous one.
/// Composition and numbers are explained in the report.
class CritterGroupGenerator {
    private:
        double spawnInterval; // seconds between critters in wave 1

    public:
        explicit CritterGroupGenerator(double spawnInterval = 0.8);

        /// Seconds between two critters of the given wave (4% shorter every wave, never below 40% of wave 1).
        double getSpawnInterval(int waveNumber) const;

        /// Throws std::invalid_argument if waveNumber < 1 or the map is not valid.
        CritterGroup generate(int waveNumber, const Map& map) const;

        static int countOfType(CritterType type, int waveNumber);
};

#endif

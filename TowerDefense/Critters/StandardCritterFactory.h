#ifndef STANDARDCRITTERFACTORY_H
#define STANDARDCRITTERFACTORY_H

#include "CritterFactory.h"
#include "CritterGroupGenerator.h"

/// The game's normal difficulty curve. Wave N contains (see the report):
/// - 3 MELEE and 3 CASTER critters,
/// - N/3 CANNON critters (one more every 3 waves),
/// - N/10 SUPER critters (one more every 10 waves),
/// so 6 + N/3 + N/10 critters, all of level N, released every baseInterval x max(1 - 0.04 (N - 1), 0.4) seconds.
/// The further the player gets, the more, stronger and faster-spawning critters the next wave has.
///
/// The composition rules live in CritterGroupGenerator (written for Assignment 1); this factory delegates to it so the
/// rules exist in one place only.
class StandardCritterFactory : public CritterFactory {
    private:
        CritterGroupGenerator generator;

    public:
        /// baseInterval: seconds between two critters in wave 1. Throws std::invalid_argument if it is not positive.
        explicit StandardCritterFactory(double baseInterval = 0.8);

        /// Throws std::invalid_argument if waveNumber < 1 or the map is not valid.
        CritterGroup createGroup(int waveNumber, const Map& map) const override;

        /// Seconds between two critters of the given wave.
        double getSpawnInterval(int waveNumber) const;

        /// Number of critters of that type in the given wave.
        static int countOfType(CritterType type, int waveNumber);
};

#endif

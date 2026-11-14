#ifndef CRITTERFACTORY_H
#define CRITTERFACTORY_H

#include "CritterGroup.h"

class Map;

/// Factory pattern (Assignment 3, Part 2): creates the group of critters of a wave.
///
/// The game only knows this interface: it asks for wave N and gets a ready CritterGroup, without knowing how many
/// critters of which type are created or how fast they spawn. A different difficulty curve (an "easy" or "endless"
/// mode, for example) is a new subclass, the game code does not change.
///
/// Every implementation must:
/// - throw std::invalid_argument if waveNumber < 1 or the map is not valid (Map::isValidMap),
/// - otherwise return a non-empty group whose getWaveNumber() is waveNumber.
class CritterFactory {
    public:
        virtual ~CritterFactory() = default;

        /// Creates the critters of wave waveNumber (1 = first wave) for the given map.
        virtual CritterGroup createGroup(int waveNumber, const Map& map) const = 0;
};

#endif

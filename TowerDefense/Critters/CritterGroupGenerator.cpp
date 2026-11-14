// Part 3 (critters): builds the critter wave for a wave number. StandardCritterFactory (Factory pattern) uses the
// same rules, so the waves of Assignment 2 and Assignment 3 are identical.
#include "CritterGroupGenerator.h"
#include "Map.h"

#include <stdexcept>

CritterGroupGenerator::CritterGroupGenerator(double spawnInterval)
: spawnInterval(spawnInterval) {
    if (spawnInterval <= 0.0) {
        throw std::invalid_argument("The spawn interval must be positive.");
    }
}

// later waves come in a tighter stream: 4% less time between two critters per wave, never below 40%
double CritterGroupGenerator::getSpawnInterval(int waveNumber) const {
    double factor = 1.0 - 0.04 * (waveNumber - 1);

    if (factor < 0.4) {
        factor = 0.4;
    }

    return spawnInterval * factor;
}

// 3 melee and 3 caster in every wave, one more cannon every 3 waves, one more super critter every 10 waves
int CritterGroupGenerator::countOfType(CritterType type, int waveNumber) {
    switch (type) {
        case CritterType::MELEE: return 3;
        case CritterType::CASTER: return 3;
        case CritterType::CANNON: return waveNumber / 3;
        case CritterType::SUPER: return waveNumber / 10;
    }
    return 0;
}

// all critters of a wave have the wave's level, so their hit points and rewards grow with the wave number
CritterGroup CritterGroupGenerator::generate(int waveNumber, const Map& map) const {
    if (waveNumber < 1) {
        throw std::invalid_argument("The wave number must be at least 1.");
    }

    if (!map.isValidMap()) {
        throw std::invalid_argument("Critters cannot be generated for an invalid map.");
    }

    std::vector<Critter> critters;

    // walking order: super critters first, casters at the back
    const CritterType order[] = {CritterType::SUPER, CritterType::MELEE, CritterType::CANNON, CritterType::CASTER};

    for (CritterType type : order) {
        for (int i = 0; i < countOfType(type, waveNumber); i++) {
            critters.push_back(Critter(type, waveNumber));
        }
    }

    return CritterGroup(waveNumber, critters, getSpawnInterval(waveNumber));
}

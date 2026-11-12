#include "CritterGroupGenerator.h"
#include "Map.h"

#include <stdexcept>

CritterGroupGenerator::CritterGroupGenerator(double spawnInterval)
: spawnInterval(spawnInterval) {
    if (spawnInterval <= 0.0) {
        throw std::invalid_argument("The spawn interval must be positive.");
    }
}

double CritterGroupGenerator::getSpawnInterval(int waveNumber) const {
    double factor = 1.0 - 0.04 * (waveNumber - 1);

    if (factor < 0.4) {
        factor = 0.4;
    }

    return spawnInterval * factor;
}

int CritterGroupGenerator::countOfType(CritterType type, int waveNumber) {
    switch (type) {
        case CritterType::MELEE: return 3;
        case CritterType::CASTER: return 3;
        case CritterType::CANNON: return waveNumber / 3;
        case CritterType::SUPER: return waveNumber / 10;
    }
    return 0;
}

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

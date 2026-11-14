#include "StandardCritterFactory.h"

StandardCritterFactory::StandardCritterFactory(double baseInterval)
: generator(baseInterval) {
}

CritterGroup StandardCritterFactory::createGroup(int waveNumber, const Map& map) const {
    // the generator checks the wave number and the map, then builds the group (level = wave number)
    return generator.generate(waveNumber, map);
}

double StandardCritterFactory::getSpawnInterval(int waveNumber) const {
    return generator.getSpawnInterval(waveNumber);
}

int StandardCritterFactory::countOfType(CritterType type, int waveNumber) {
    return CritterGroupGenerator::countOfType(type, waveNumber);
}

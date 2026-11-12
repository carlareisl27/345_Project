#include "CritterGroup.h"
#include "Map.h"

#include <algorithm>

CritterGroup::CritterGroup(int waveNumber, const std::vector<Critter>& critters, double spawnInterval)
: waveNumber(waveNumber), critters(critters), spawnInterval(spawnInterval),
elapsed(0.0), nextToSpawn(0), killed(0), escaped(0), coinsEarned(0), coinsStolen(0) {
}

void CritterGroup::update(double deltaTime, const Map& map, int& coins) {
    if (deltaTime < 0.0) {
        return;
    }

    elapsed += deltaTime;

    // what the observers display of every critter, to notify them only when something really changed in this update
    struct Snapshot { int row; int column; int hitPoints; CritterState state; };
    std::vector<Snapshot> before;
    before.reserve(critters.size());
    for (const Critter& critter : critters) {
        before.push_back({critter.getRow(), critter.getColumn(), critter.getHitPoints(), critter.getState()});
    }
    const int coinsBefore = coinsEarned - coinsStolen;

    // one game element per cell: true if an active critter of this wave is on that cell.
    auto isOccupied = [this](int row, int column) {
        for (const Critter& other : critters) {
            if (other.isActive() && other.getRow() == row && other.getColumn() == column) {
                return true;
            }
        }
        return false;
    };

    // critters never pass each other, so the order of the vector is front first: the critter in front moves
    // first and frees its cell for the one behind it. Critters that spawn in this update move after it.
    for (int i = 0; i < nextToSpawn; i++) {
        critters[i].update(deltaTime, isOccupied);
    }

    // critter i spawns at i * spawnInterval, or as soon as the entry is free if the one before is still on it.
    std::pair<int, int> entry = map.getEntry();
    int firstNew = nextToSpawn;
    while (nextToSpawn < static_cast<int>(critters.size()) && elapsed >= nextToSpawn * spawnInterval
           && !isOccupied(entry.first, entry.second)) {
        critters[nextToSpawn].spawn(map);
        nextToSpawn++;
    }

    for (int i = firstNew; i < nextToSpawn; i++) {
        critters[i].update(deltaTime, isOccupied);
    }

    for (Critter& critter : critters) {
        int reward = critter.claimReward();
        if (reward > 0) {
            coins += reward;
            coinsEarned += reward;
            killed++;
        }

        if (critter.hasEscaped()) {
            int penalty = critter.claimPenalty();
            if (penalty > 0) {
                int stolen = std::min(coins, penalty);
                coins -= stolen;
                coinsStolen += stolen;
                escaped++;
            }
        }
    }

    bool changed = (coinsEarned - coinsStolen) != coinsBefore;
    for (std::size_t i = 0; i < critters.size() && !changed; i++) {
        const Critter& critter = critters[i];
        changed = critter.getRow() != before[i].row || critter.getColumn() != before[i].column
               || critter.getHitPoints() != before[i].hitPoints || critter.getState() != before[i].state;
    }

    if (changed) {
        notify(); // observer pattern: the wave changed, its views are refreshed
    }
}

bool CritterGroup::addCritter(const Critter& critter) {
    if (critter.getState() != CritterState::WAITING) {
        return false;
    }

    critters.push_back(critter);
    notify();
    return true;
}

bool CritterGroup::removeCritter(int index) {
    if (index < 0 || index >= static_cast<int>(critters.size())) {
        return false;
    }

    // a critter that already spawned is removed from the spawned ones too, so the next critter to spawn stays the same.
    if (index < nextToSpawn) {
        nextToSpawn--;
    }

    critters.erase(critters.begin() + index);
    notify();
    return true;
}

std::vector<Critter*> CritterGroup::getActiveCritters() {
    std::vector<Critter*> active;

    for (Critter& critter : critters) {
        if (critter.isActive()) {
            active.push_back(&critter);
        }
    }

    return active;
}

const std::vector<Critter>& CritterGroup::getCritters() const {
    return critters;
}

bool CritterGroup::isFinished() const {
    if (nextToSpawn < static_cast<int>(critters.size())) {
        return false;
    }

    for (const Critter& critter : critters) {
        if (critter.isActive()) {
            return false;
        }
    }

    return true;
}

int CritterGroup::getWaveNumber() const {
    return waveNumber;
}

double CritterGroup::getSpawnInterval() const {
    return spawnInterval;
}

int CritterGroup::getSize() const {
    return static_cast<int>(critters.size());
}

int CritterGroup::getTotalHitPoints() const {
    int total = 0;

    for (const Critter& critter : critters) {
        total += critter.getMaxHitPoints();
    }

    return total;
}

int CritterGroup::getSpawnedCount() const {
    return nextToSpawn;
}

int CritterGroup::getKilledCount() const {
    return killed;
}

int CritterGroup::getEscapedCount() const {
    return escaped;
}

int CritterGroup::getCoinsEarned() const {
    return coinsEarned;
}

int CritterGroup::getCoinsStolen() const {
    return coinsStolen;
}

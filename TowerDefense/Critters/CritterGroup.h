#ifndef CRITTERGROUP_H
#define CRITTERGROUP_H

#include "Critter.h"

#include <vector>

class Map;

/// The critters of one wave. Releases them one after the other on the entry, moves them and settles the coins:
/// the reward of every killed critter is added, the coins stolen by every critter that reached the exit are taken.
/// Do not copy the group while towers hold pointers from getActiveCritters.
/// CritterGroup is Observable: adding or removing a critter, and every update in which a critter of the wave spawns,
/// changes cell, loses hit points, dies or escapes, call notify(), so the attached observers (CritterGroupObserver)
/// re-display the wave.
class CritterGroup : public Observable {
    private:
        int waveNumber;
        std::vector<Critter> critters;
        double spawnInterval;
        double elapsed;
        int nextToSpawn;

        int killed;
        int escaped;
        int coinsEarned;
        int coinsStolen;

    public:
        CritterGroup(int waveNumber, const std::vector<Critter>& critters, double spawnInterval);

        /// Spawns, moves and settles coins. The first critter spawns in the first update.
        void update(double deltaTime, const Map& map, int& coins);

        /// Critters on the map that are alive: the ones towers can shoot at.
        std::vector<Critter*> getActiveCritters();

        const std::vector<Critter>& getCritters() const;

        /// Adds a critter at the end of the wave (it spawns after the others). Its state must be WAITING, otherwise false.
        /// Adding or removing critters invalidates the pointers returned by getActiveCritters.
        bool addCritter(const Critter& critter);

        /// Removes the critter at that position (0 = first critter of the wave). False if the index is out of range.
        bool removeCritter(int index);

        /// True when every critter has spawned and none is left on the map.
        bool isFinished() const;

        int getWaveNumber() const;
        double getSpawnInterval() const;
        int getSize() const;
        int getTotalHitPoints() const; ///< sum of the maximum hit points, a measure of the wave difficulty
        int getSpawnedCount() const;
        int getKilledCount() const;
        int getEscapedCount() const;
        int getCoinsEarned() const;
        int getCoinsStolen() const;
};

#endif

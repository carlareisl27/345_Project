#ifndef CRITTER_H
#define CRITTER_H

#include <functional>
#include <utility>
#include <vector>

#include "Observer.h" // observer pattern (Part 3 of the observer assignment)

class Map;

/// Kinds of critters, taken from the lane minions of League of Legends.
enum class CritterType {
    MELEE,
    CASTER,
    CANNON,
    SUPER,
};

enum class CritterState {
    WAITING, ///< created but not on the map yet
    ACTIVE,  ///< walking towards the exit
    DEAD,
    ESCAPED, ///< reached the exit
};

/// One critter: reward, hit points, strength, speed and level. Stats come from the type and the level (see the report).
/// It finds the shortest path to the exit when it spawns and then follows it.
/// Critter is Observable: spawning, moving to another cell, losing hit points, being slowed (and the slow wearing off),
/// dying and escaping call notify(), so the attached observers (CritterObserver) re-display the critter.
class Critter : public Observable {
    private:
        CritterType type;
        CritterState state;

        int level;
        int maxHitPoints;
        int hitPoints;
        int strength;
        double speed; // cells per second
        int reward;

        double slowFactor;
        double slowTimer;

        std::vector<std::pair<int, int>> path;
        int pathIndex;
        double progress; // part of the next step already walked (0 to 1)

        bool rewardClaimed;
        bool penaltyClaimed;

        bool findPath(const Map& map);

    public:
        Critter(CritterType type, int level);

        /// Puts the critter on the entry. False if it already spawned or the map has no path to the exit.
        bool spawn(const Map& map);

        /// The cell the critter decides to move to next (the exit itself once it is there).
        std::pair<int, int> chooseNextCell() const;

        /// Moves the critter along its path.
        /// isOccupied(row, column) tells whether another critter is on that cell. When it is, the critter
        /// waits just behind it until the cell is free, so a path cell never holds two critters
        /// (one game element per cell). The exit is never blocked: a critter that reaches it leaves the map.
        /// Without isOccupied the critter walks freely (used for a single critter).
        void update(double deltaTime, const std::function<bool(int, int)>& isOccupied = nullptr);

        /// True if this hit killed the critter. Only critters on the map can be hit.
        bool takeDamage(int amount);

        /// Multiplies the speed by factor (0 to 1) for duration seconds.
        void applySlow(double factor, double duration);

        /// Reward of a killed critter, given once (0 afterwards or if it is not dead).
        int claimReward();

        /// Coins stolen by a critter that reached the exit, given once.
        int claimPenalty();

        int getRow() const;    ///< -1 before spawning
        int getColumn() const; ///< -1 before spawning
        bool isAlive() const;
        bool isActive() const;
        bool hasEscaped() const;
        CritterType getType() const;
        CritterState getState() const;
        int getLevel() const;
        int getHitPoints() const;
        int getMaxHitPoints() const;
        int getStrength() const;
        int getReward() const;
        double getSpeed() const;
        double getCurrentSpeed() const; ///< lower than getSpeed while slowed
        int getPathLength() const;
        int getPathIndex() const;       ///< cells already walked on the path, -1 before spawning
        double getSlowTimeLeft() const; ///< seconds of slow left, 0 when not slowed
        /// Smooth position in cell units for drawing between path cells.
        std::pair<double, double> getExactPosition() const;
        const char* getName() const;
        char getSymbol() const; ///< M, C, N or S, used to draw the critter in the console
};

#endif

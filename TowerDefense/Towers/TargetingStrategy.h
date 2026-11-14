#ifndef TARGETINGSTRATEGY_H
#define TARGETINGSTRATEGY_H

#include <vector>

class Critter;

/// Strategy pattern (Assignment 3, Part 1): how a tower chooses which critter to shoot.
///
/// A Tower owns one TargetingStrategy. Before calling selectTarget, the tower filters the critters so the strategy only
/// receives candidates that are non-null, alive and inside the tower's range; the strategy only decides which one wins.
/// Swapping the strategy (Tower::setTargetingStrategy) changes the tower's behaviour at run time without touching the
/// Tower class hierarchy.
///
/// Concrete strategies: NearestToTowerStrategy, NearestToExitStrategy, StrongestCritterStrategy, WeakestCritterStrategy.
/// On a tie every strategy keeps the first candidate in the vector (stable tie-break).
class TargetingStrategy {
    public:
        virtual ~TargetingStrategy() = default;

        /// Returns one of the candidates, or nullptr if candidates is empty.
        virtual Critter* selectTarget(const std::vector<Critter*>& candidates) const = 0;

        /// Short name of the strategy ("Nearest", "Exit", "Strongest", "Weakest"), shown in the GUI.
        virtual const char* getName() const = 0;
};

#endif

#ifndef NEARESTTOEXITSTRATEGY_H
#define NEARESTTOEXITSTRATEGY_H

#include "TargetingStrategy.h"

/// Targets the candidate closest to the exit: the one that walked the most path cells (highest Critter::getPathIndex).
/// Every critter follows the shortest path from the entry to the exit, so more cells walked means fewer cells left.
class NearestToExitStrategy : public TargetingStrategy {
    public:
        Critter* selectTarget(const std::vector<Critter*>& candidates) const override;
        /// Short name shown in the GUI.
        const char* getName() const override { return "Exit"; }
};

#endif

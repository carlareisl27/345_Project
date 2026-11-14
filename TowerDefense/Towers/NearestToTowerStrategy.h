#ifndef NEARESTTOTOWERSTRATEGY_H
#define NEARESTTOTOWERSTRATEGY_H

#include "TargetingStrategy.h"

class Tower;

/// Targets the candidate with the smallest Euclidean distance to the tower's cell. This is the default strategy of every
/// tower (the behaviour towers had before the Strategy pattern was added).
class NearestToTowerStrategy : public TargetingStrategy {
    private:
        const Tower* tower; // tower whose cell is used, read on every call so the strategy follows Tower::moveTo
        int fixedRow;       // used instead when no tower is given
        int fixedColumn;

    public:
        /// Measures distances from the current cell of the given tower (the tower is not owned).
        explicit NearestToTowerStrategy(const Tower* tower);

        /// Measures distances from a fixed cell (useful when testing the strategy without a tower).
        NearestToTowerStrategy(int row, int column);

        Critter* selectTarget(const std::vector<Critter*>& candidates) const override;
        /// Short name shown in the GUI.
        const char* getName() const override { return "Nearest"; }
};

#endif

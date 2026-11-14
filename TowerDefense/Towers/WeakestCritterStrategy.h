#ifndef WEAKESTCRITTERSTRATEGY_H
#define WEAKESTCRITTERSTRATEGY_H

#include "TargetingStrategy.h"

/// Targets the candidate with the fewest hit points left (Critter::getHitPoints), to finish critters off quickly.
class WeakestCritterStrategy : public TargetingStrategy {
    public:
        Critter* selectTarget(const std::vector<Critter*>& candidates) const override;
        /// Short name shown in the GUI.
        const char* getName() const override { return "Weakest"; }
};

#endif

#ifndef STRONGESTCRITTERSTRATEGY_H
#define STRONGESTCRITTERSTRATEGY_H

#include "TargetingStrategy.h"

/// Targets the candidate with the most hit points left (Critter::getHitPoints).
class StrongestCritterStrategy : public TargetingStrategy {
    public:
        Critter* selectTarget(const std::vector<Critter*>& candidates) const override;
        /// Short name shown in the GUI.
        const char* getName() const override { return "Strongest"; }
};

#endif

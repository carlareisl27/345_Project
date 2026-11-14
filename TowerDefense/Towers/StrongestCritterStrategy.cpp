#include "StrongestCritterStrategy.h"
#include "Critter.h"

Critter* StrongestCritterStrategy::selectTarget(const std::vector<Critter*>& candidates) const {
    Critter* best = nullptr;

    for (Critter* critter : candidates) {
        if (critter == nullptr) {
            continue;
        }

        // strict '>': on a tie the first candidate stays selected
        if (best == nullptr || critter->getHitPoints() > best->getHitPoints()) {
            best = critter;
        }
    }

    return best;
}

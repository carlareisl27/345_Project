#include "NearestToExitStrategy.h"
#include "Critter.h"

Critter* NearestToExitStrategy::selectTarget(const std::vector<Critter*>& candidates) const {
    Critter* best = nullptr;

    for (Critter* critter : candidates) {
        if (critter == nullptr) {
            continue;
        }

        // strict '>': on a tie the first candidate stays selected
        if (best == nullptr || critter->getPathIndex() > best->getPathIndex()) {
            best = critter;
        }
    }

    return best;
}

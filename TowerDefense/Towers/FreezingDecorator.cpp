// Decorator pattern (Assignment 3, Part 3): the Freezing upgrade. Every hit also slows the target down.
#include "FreezingDecorator.h"
#include "Critter.h"

#include <algorithm>

// out-of-range values are corrected instead of refused: the factor is clamped to [0, 1], a duration <= 0 becomes 0.01 s
FreezingDecorator::FreezingDecorator(Tower* tower, double slowFactor, double slowDuration)
: TowerDecorator(tower),
  slowFactor(std::clamp(slowFactor, 0.0, 1.0)),
  slowDuration(slowDuration <= 0.0 ? 0.01 : slowDuration) {
}

double FreezingDecorator::getSlowFactor() const {
    return slowFactor;
}

double FreezingDecorator::getSlowDuration() const {
    return slowDuration;
}

const char* FreezingDecorator::getDecoratorLabel() const {
    return "Freezing";
}

// the wrapped tower hits first (with every decorator under this one), then the target is slowed
void FreezingDecorator::applyAttack(Critter* target, const std::vector<Critter*> & critters) {
    TowerDecorator::applyAttack(target, critters);
    target->applySlow(slowFactor, slowDuration);
}

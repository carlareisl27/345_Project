#ifndef FREEZINGDECORATOR_H
#define FREEZINGDECORATOR_H

#include "TowerDecorator.h"

/// Adds a freezing effect to any tower: after the wrapped attack, the target is slowed with Critter::applySlow.
/// A new hit on a slowed critter restarts the slow at the configured duration (it is not added to the time left).
class FreezingDecorator : public TowerDecorator {
    private:
        double slowFactor;   // the critter keeps this part of its speed (0.5 = half speed)
        double slowDuration; // seconds

    protected:
        void applyAttack(Critter* target, const std::vector<Critter*> & critters) override;

    public:
        /// Wraps the tower (owned). The factor is clamped to [0, 1] and a duration <= 0 becomes 0.01 s (no exception).
        explicit FreezingDecorator(Tower* tower, double slowFactor = 0.5, double slowDuration = 2.0);

        double getSlowFactor() const;
        double getSlowDuration() const;
        const char* getDecoratorLabel() const override;
};

#endif

#ifndef SPLASHDECORATOR_H
#define SPLASHDECORATOR_H

#include "TowerDecorator.h"

/// Adds splash damage to any tower: after the wrapped attack, every other active critter within splashRadius cells of
/// the target takes 50% of the wrapped tower's damage (at least 1). The splash happens even if the target died.
class SplashDecorator : public TowerDecorator {
    private:
        const double splashRadius;

    protected:
        void applyAttack(Critter* target, const std::vector<Critter*> & critters) override;

    public:
        /// Wraps the tower (owned). A negative radius is treated as 0.
        explicit SplashDecorator(Tower* tower, double splashRadius = 1.5);

        double getSplashRadius() const;
        /// Damage dealt to each critter around the target: half the wrapped tower's damage, at least 1.
        int getSplashDamage() const;
        const char* getDecoratorLabel() const override;
};

#endif

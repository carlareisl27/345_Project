#ifndef BURNINGDECORATOR_H
#define BURNINGDECORATOR_H

#include "TowerDecorator.h"

#include <map>

/// Adds a burning effect to any tower: a critter hit (and still alive) keeps taking damage after the hit.
/// Each tick deals floor(hit damage x tickDamagePercent) (at least 1), once every tickInterval seconds, tickCount times.
/// Hitting a burning critter again restarts its ticks (count and timer) but keeps its per-tick damage, so a critter
/// never has more than one burn at a time.
/// The burns are advanced every time the decorator is updated (update(), or the update of an outer decorator), and only
/// for critters present in the list passed to that update, so a critter that left the game is never touched.
class BurningDecorator : public TowerDecorator {
    private:
        struct BurnState {
            int tickDamage;
            int ticksLeft;
            double timeSinceLastTick;
        };

        double tickDamagePercent;
        int tickCount;
        double tickInterval;
        std::map<const Critter*, BurnState> burns;

        void advanceBurns(double deltaTime, const std::vector<Critter*> & critters);

    protected:
        void applyAttack(Critter* target, const std::vector<Critter*> & critters) override;
        bool performUpdate(double deltaTime, const std::vector<Critter*> & critters, Tower& attacker) override;

    public:
        /// Wraps the tower (owned). Out-of-range values are corrected: percent < 0 -> 0, count < 1 -> 1,
        /// interval <= 0 -> 0.01 s.
        explicit BurningDecorator(Tower* tower, double tickDamagePercent = 0.20, int tickCount = 3, double tickInterval = 1.0);

        double getTickDamagePercent() const;
        int getTickCount() const;
        double getTickInterval() const;

        bool isBurning(const Critter* critter) const;
        int getBurnTicksLeft(const Critter* critter) const;  ///< 0 if the critter is not burning
        int getBurnTickDamage(const Critter* critter) const; ///< 0 if the critter is not burning
        int getBurningCount() const;

        const char* getDecoratorLabel() const override;
};

#endif

#ifndef LEVELUPDECORATOR_H
#define LEVELUPDECORATOR_H

#include "TowerDecorator.h"

/// Makes the wrapped tower go up one level. This is how a tower is upgraded during play (see levelUp in TowerUpgrades.h):
/// every upgrade wraps the tower in one more LevelUpDecorator.
/// - level: the wrapped tower's level + 1 (the name gets a matching suffix: "Basic Tower Lv2", "Basic Tower Lv3")
/// - damage and range x1.2 (damage rounded down), fire rate x1.1. Stacking two gives x1.44 damage and range.
/// - refund: the wrapped tower's refund plus half of the price paid for this level
/// - upgrade() is refused: the plain Tower::upgrade must never run under a level-up decorator, or the level would be
///   counted twice. Use levelUp() instead.
class LevelUpDecorator : public TowerDecorator {
    private:
        int pricePaid; // coins paid for this level (0 when the decorator is created directly, for example in a test)

    protected:
        /// Runs the wrapped attack, then deals the extra damage of this level to the target if it is still alive.
        void applyAttack(Critter* target, const std::vector<Critter*> & critters) override;

        int getLevelsAdded() const override;

    public:
        /// Wraps the tower (owned) and notifies the observers so they see the new stats immediately.
        /// pricePaid is what the player paid for this level (a negative price is treated as 0).
        explicit LevelUpDecorator(Tower* tower, int pricePaid = 0);

        /// Always false: a tower is upgraded by wrapping it in another LevelUpDecorator (levelUp), never by Tower::upgrade.
        bool upgrade(int& coins) override;

        int getLevel() const override;
        int getDamage() const override;
        double getRange() const override;
        double getFireRate() const override;
        int getRefundValue() const override;
        int getPricePaid() const;
        const char* getName() const override;
        const char* getDecoratorLabel() const override;
};

#endif

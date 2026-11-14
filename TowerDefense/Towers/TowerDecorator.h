#ifndef TOWERDECORATOR_H
#define TOWERDECORATOR_H

#include "Tower.h"

#include <string>

/// Decorator pattern (Assignment 3, Part 3): base class of every tower upgrade that can be added during play.
///
/// A TowerDecorator IS a Tower (so the game, the GUI and the observers use it like any other tower) and HAS a Tower:
/// the wrapped tower, which it owns and deletes. Every Tower operation is forwarded to the wrapped tower, so a decorator
/// that adds nothing behaves exactly like the tower it wraps. Concrete decorators override only what they change:
/// - LevelUpDecorator: one level higher, +20% damage and range, +10% fire rate (stats)
/// - SplashDecorator: damage also hits the critters around the target
/// - BurningDecorator: the target keeps taking damage for a few seconds
/// - FreezingDecorator: the target is slowed down
///
/// Decorators stack: new BurningDecorator(new SplashDecorator(new LevelUpDecorator(new BasicTower(1, 1)))).
/// On each shot the innermost tower handles the cooldown and target selection, then calls applyAttack on the OUTERMOST
/// decorator, which runs the wrapped attack first and then adds its own effect, all the way down the chain.
///
/// Mutating operations (purchase, upgrade, sell, moveTo, update) call notify() on the decorator after forwarding, so the
/// observers attached to the decorator see the new state of the wrapped tower.
class TowerDecorator : public Tower {
    private:
        mutable std::string nameCache; // storage for the string returned by getName

        // throws std::invalid_argument for a null tower (used before the Tower base is built)
        static Tower* requireTower(Tower* tower);

    protected:
        Tower* wrapped; // owned: deleted by the destructor

        // forwarded to the wrapped tower (see Tower)
        void applyAttack(Critter* target, const std::vector<Critter*> & critters) override;
        Critter* selectTargetFor(const std::vector<Critter*> & critters, const Tower& rangeOwner) const override;
        bool performUpdate(double deltaTime, const std::vector<Critter*> & critters, Tower& attacker) override;

        /// Number of levels this decorator adds to the name ("Lv2", "Lv3"...). 0 except for LevelUpDecorator.
        virtual int getLevelsAdded() const;

    public:
        /// Wraps the tower and takes ownership of it. Throws std::invalid_argument if tower is nullptr.
        explicit TowerDecorator(Tower* tower);

        /// Deletes the wrapped tower (and therefore the whole chain under it).
        ~TowerDecorator() override;

        /// Short label of the effect added by this decorator ("Level Up", "Splash", "Burning", "Freezing").
        /// Pure virtual: TowerDecorator itself is abstract, only concrete decorators are created.
        virtual const char* getDecoratorLabel() const = 0;

        /// The tower directly wrapped by this decorator (still owned by the decorator).
        const Tower* getWrappedTower() const;

        /// Base tower name, a level suffix if level-up decorators are present, and one "+Label" per other decorator,
        /// in the order they were applied. For example "Basic Tower Lv2 +Splash +Burning".
        const char* getName() const override;

        bool purchase(int& coins) override;
        bool upgrade(int& coins) override;
        /// Refunds getRefundValue() of this decorator (so every decorator's share is paid) and marks the chain sold.
        int sell(int& coins) override;
        bool moveTo(int newRow, int newColumn) override;
        bool update(double deltaTime, const std::vector<Critter*> & critters) override;
        void setTargetingStrategy(std::unique_ptr<TargetingStrategy> strategy) override;
        const TargetingStrategy& getTargetingStrategy() const override;

        /// Uses this decorator's getRow, getColumn and getRange, so a decorator that changes the range is taken into account.
        bool isInRange(int targetRow, int targetColumn) const override;

        int getRow() const override;
        int getColumn() const override;
        int getDamage() const override;
        int getLevel() const override;
        int getCost() const override;
        /// Price of the next level: getCost() x getLevel(), 0 when not purchased, sold or at the maximum level.
        int getUpgradeCost() const override;
        int getMaxLevel() const override;
        int getRefundValue() const override;
        double getRange() const override;
        double getFireRate() const override;
        bool isPurchased() const override;
        bool isSold() const override;
        double getCooldown() const override;
};

#endif

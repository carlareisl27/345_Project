#ifndef TOWER_H
#define TOWER_H

// include vector library be able to store critter pointers
#include <vector>
#include <memory>

// Observer pattern (Part 1 of the observer assignment): a Tower is an Observable, a TowerObserver displays it.
#include "Observer.h"

// Strategy pattern (Assignment 3, Part 1): every tower owns a TargetingStrategy that chooses its target.
#include "TargetingStrategy.h"

// implementation of the critter class
class Critter;

// Decorator pattern (Assignment 3, Part 3): a TowerDecorator forwards every call to the tower it wraps,
// including the protected applyAttack / performUpdate / selectTargetFor, so it is declared a friend.
class TowerDecorator;

// Tower is Observable: purchase, upgrade, sell, moveTo, every shot (cooldown reset) and every change of targeting
// strategy call notify(), so the attached observers (TowerObserver) re-display the tower each time one of its values changes.
//
// The public operations are virtual so that a TowerDecorator, which is itself a Tower, can be used anywhere a Tower is
// expected (Decorator pattern), and can change what the getters report (for example LevelUpDecorator::getDamage).
class Tower : public Observable {
    friend class TowerDecorator;

    // protected members that can only be accessed by Tower and derived classes
    protected:
    // attributes for each tower
    int row;
    int column;
    int cost;
    int damage;
    double range;
    double fireRate;
    int level;
    int maxLevel; // highest level a tower can reach
    int investments;
    double cooldown; // time remaining until a tower can attack again
    bool purchased;
    bool sold;

    // targeting strategy (Strategy pattern), a NearestToTowerStrategy until setTargetingStrategy is called
    std::unique_ptr<TargetingStrategy> targetingStrategy;

    // function updateCost calculates the cost for upgrading a tower
    int updateCost() const;

    // keeps the critters that can be shot (non-null, alive, in the range of rangeOwner) and lets the targeting strategy
    // choose one of them. rangeOwner is normally the tower itself, it is the outermost decorator when the tower is
    // decorated, so that a decorator that increases the range (LevelUpDecorator) also widens the targeting.
    virtual Critter* selectTargetFor(const std::vector<Critter*> & critters, const Tower& rangeOwner) const;

    // one tick of the tower: cooldown, target selection, attack. 'attacker' is the object whose applyAttack is used:
    // the tower itself, or the outermost decorator, so every decorator in the chain adds its effect to the shot.
    virtual bool performUpdate(double deltaTime, const std::vector<Critter*> & critters, Tower& attacker);

    // uses an abstract attrack operation for each tower, 'target' acts as the critter to attack and 'critters' has the critters
    // available to attack.
    virtual void applyAttack(Critter* target, const std::vector<Critter*> & critters) = 0;

    // memebers that can be accessed from outside the class
    public:
    // initializes a tower with its corresponding attributes
    Tower(int row, int column, int cost, int damage, double range, double fireRate);

    // this is a virtual destructor that allows for other objects to be destroyed
    virtual ~Tower() = default;

    // a tower owns its strategy and its observers watch one object: towers are not copied
    Tower(const Tower&) = delete;
    Tower& operator=(const Tower&) = delete;

    virtual bool purchase(int& coins); // to purchase a tower using coins that the user currently has
    virtual bool upgrade(int& coins); // to upgrade a tower using the available balance of coins from the user
    virtual int sell(int& coins); // to sell a tower and add the price given for it to the users coins

    /// Moves the tower to another cell (for example after dragging it on the map). False if the tower is sold.
    /// The game checks that the new cell is a scenery cell (Map::isSceneryCell), the tower only stores it.
    virtual bool moveTo(int newRow, int newColumn);

    virtual bool isInRange(int targetRow, int targetColumn) const; // checks if a positions is inside the attack range of a tower
    // if it is, it returns true, false otherwise.

    // to update a tower temporarily, helps for cooldown, or attacking and finding targets
    virtual bool update(double deltaTime, const std::vector<Critter*> & critters);

    /// Strategy pattern: replaces the targeting strategy used for every following shot and notifies the observers.
    /// Passing nullptr restores the default NearestToTowerStrategy.
    virtual void setTargetingStrategy(std::unique_ptr<TargetingStrategy> strategy);

    /// The targeting strategy used now (never null). A decorated tower returns the strategy of the tower it wraps.
    virtual const TargetingStrategy& getTargetingStrategy() const;

    /// The critter this tower would shoot now: the critters are filtered (non-null, alive, in range) and the targeting
    /// strategy chooses among them. nullptr if none can be shot. Used by update() and by the GUI to preview the target.
    Critter* selectTarget(const std::vector<Critter*> & critters) const;

    // getters
    virtual int getRow() const;
    virtual int getColumn() const;
    virtual int getDamage() const;
    virtual int getLevel() const;
    virtual int getCost() const;
    /// Upgrade cost for the current level, or 0 if the tower cannot be upgraded now.
    virtual int getUpgradeCost() const;
    /// Highest level this tower can reach.
    virtual int getMaxLevel() const;
    virtual int getRefundValue() const; // coins the player gets back if the tower is sold now
    virtual double getRange() const;
    virtual double getFireRate() const;
    virtual bool isPurchased() const;
    virtual bool isSold() const;
    virtual double getCooldown() const; // seconds left before the tower can shoot again

    // each tower has to provide a name but it is only virtual which means each tower is abstract
    virtual const char* getName() const = 0;

};

// Basic Tower class represents the lowest level of tower
class BasicTower : public Tower {
    protected:
    // attack operation, it overrides the virtual function of the tower
    void applyAttack(Critter* target, const std::vector<Critter*> & critters) override;

    // to create a basic tower in the given map position
    public:
    BasicTower(int row, int column);

    // the name of the tower gets returned
    const char* getName() const override;

};

// Splash Tower class represents the next level of tower, this type can damage any critter that is close to its radius.
class SplashTower : public Tower {
    private:
    // radius at which a critter can get damaged
    double splashRadius;

    protected:
    // attack operation
    void applyAttack(Critter* target, const std::vector<Critter*> & critters) override;

    // to create a splash tower in the given map position
    public:
    SplashTower(int row, int column);

    double getSplashRadius() const; // cells around the target that also take damage

    const char* getName() const override;
};

// Slow Tower class represents a tower that can make the critters move slower
class SlowTower : public Tower {
    private:
    // factor at which the critter's speed is reduced
    double slowFactor;
    // how long it slows down the critter for
    double slowDuration;

    protected:
    // attack operation
    void applyAttack(Critter* target, const std::vector<Critter*> & critters) override;

    // to create a slow tower in the given map position
    public:
    SlowTower(int row, int column);

    double getSlowFactor() const;   // the critter keeps this part of its speed (0.5 = half speed)
    double getSlowDuration() const; // seconds the slow lasts

    const char* getName() const override;
};

#endif

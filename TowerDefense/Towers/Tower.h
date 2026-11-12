#ifndef TOWER_H
#define TOWER_H

// include vector library be able to store critter pointers
#include <vector>

// Observer pattern (Part 1 of the observer assignment): a Tower is an Observable, a TowerObserver displays it.
#include "Observer.h"

// implementation of the critter class
class Critter;

// Tower is Observable: purchase, upgrade, sell, moveTo and every shot (cooldown reset) call notify(),
// so the attached observers (TowerObserver) re-display the tower each time one of its values changes.
class Tower : public Observable {
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

    // function updateCost calculates the cost for upgrading a tower
    int updateCost() const;

    // checks for the critters that can be used and selects the target
    Critter* selectTarget(const std::vector<Critter*> & critters) const;

    // uses an abstract attrack operation for each tower, 'target' acts as the critter to attack and 'critters' has the critters
    // available to attack.
    virtual void applyAttack(Critter* target, const std::vector<Critter*> & critters) = 0;

    // memebers that can be accessed from outside the class
    public:
    // initializes a tower with its corresponding attributes
    Tower(int row, int column, int cost, int damage, double range, double fireRate);

    // this is a virtual destructor that allows for other objects to be destroyed
    virtual ~Tower() = default;

    bool purchase(int& coins); // to purchase a tower using coins that the user currently has
    bool upgrade(int& coins); // to upgrade a tower using the available balance of coins from the user
    int sell(int& coins); // to sell a tower and add the price given for it to the users coins

    /// Moves the tower to another cell (for example after dragging it on the map). False if the tower is sold.
    /// The game checks that the new cell is a scenery cell (Map::isSceneryCell), the tower only stores it.
    bool moveTo(int newRow, int newColumn);

    bool isInRange(int targetRow, int targetColumn) const; // checks if a positions is inside the attack range of a tower
    // if it is, it returns true, false otherwise.

    // to update a tower temporarily, helps for cooldown, or attacking and finding targets
    bool update(double deltaTime, const std::vector<Critter*> & critters);

    // getters
    int getRow() const;
    int getColumn() const;
    int getDamage() const;
    int getLevel() const;
    int getCost() const;
    /// Upgrade cost for the current level, or 0 if the tower cannot be upgraded now.
    int getUpgradeCost() const;
    /// Highest level this tower can reach.
    int getMaxLevel() const;
    int getRefundValue() const; // coins the player gets back if the tower is sold now
    double getRange() const;
    double getFireRate() const;
    bool isPurchased() const;
    bool isSold() const;
    double getCooldown() const; // seconds left before the tower can shoot again

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
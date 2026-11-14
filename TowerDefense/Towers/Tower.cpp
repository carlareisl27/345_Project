// Part 2 (towers): the Tower base class and the three concrete towers (Basic, Splash, Slow).
// Every change of state calls notify() for the observers (Observer pattern), and the target is chosen by the
// tower's TargetingStrategy (Strategy pattern, Assignment 3).
#include "Tower.h"
#include "Critter.h"
#include "NearestToTowerStrategy.h"

#include <cmath>
#include <utility>

// a new tower starts at level 1, not bought yet, ready to shoot, with the nearest-critter strategy
Tower::Tower(int row, int column, int cost, int damage, double range, double fireRate)
: row(row),
  column(column),
  cost(cost),
  damage(damage),
  range(range),
  fireRate(fireRate),
  level(1),
  maxLevel(3),
  investments(cost),
  cooldown(0.0),
  purchased(false),
  sold(false),
  targetingStrategy(std::make_unique<NearestToTowerStrategy>(this)) { // default strategy: nearest critter to the tower

}

// the price of the next level grows with the level: cost x current level
int Tower::updateCost() const{
    return cost*level;
}

// takes the price if the player can pay for it. A tower can be bought only once.
bool Tower::purchase(int & coins){
    if(purchased || sold || coins < cost){
        return false;
    }

    coins -= cost;
    purchased = true;

    notify(); // observer pattern: the tower changed, its views are refreshed
    return true;
}

// Assignment 1 upgrade, kept for the Assignment 1 and observer tests. During play towers are upgraded with
// levelUp() (TowerUpgrades.h), which wraps them in a LevelUpDecorator instead.
bool Tower::upgrade(int & coins){
    if(!purchased || sold || level >= maxLevel){
        return false;
    }

    int upgradeCost = updateCost();

    if(coins < upgradeCost){
        return false;
    }

    coins -= upgradeCost;
    investments += upgradeCost;
    level ++;

    damage = static_cast<int>(std::ceil(damage * 1.25));
    range += 0.5;
    fireRate *= 1.10;

    notify();
    return true;
}

// gives back the refund (half of what was invested) and takes the tower off the map
int Tower::sell(int & coins){
    if(!purchased || sold){
        return 0;
    }

    int refund = getRefundValue();

    coins += refund;
    sold = true;
    purchased = false;

    notify();
    return refund;
}

bool Tower::moveTo(int newRow, int newColumn){
    if(sold){
        return false;
    }

    // same cell: nothing changes, so the observers are not told anything
    if(newRow == row && newColumn == column){
        return true;
    }

    row = newRow;
    column = newColumn;

    notify();
    return true;
}

// straight-line distance from the tower's cell to the target's cell, compared with the range
bool Tower::isInRange(int targetRow, int targetColumn) const {
    double rowDiff = targetRow - row;
    double columnDiff = targetColumn - column;

    double distance = std::sqrt(rowDiff * rowDiff + columnDiff * columnDiff);

    return distance <= range;
}

void Tower::setTargetingStrategy(std::unique_ptr<TargetingStrategy> strategy){
    if(strategy == nullptr){
        strategy = std::make_unique<NearestToTowerStrategy>(this);
    }

    targetingStrategy = std::move(strategy);

    notify(); // the tower now behaves differently, its views are refreshed
}

const TargetingStrategy& Tower::getTargetingStrategy() const {
    return *targetingStrategy;
}

Critter* Tower::selectTarget(const std::vector<Critter*> & critters) const {
    return selectTargetFor(critters, *this);
}

// Strategy pattern: this method only builds the list of candidates, the choice itself is made by the strategy.
Critter* Tower::selectTargetFor(const std::vector<Critter*> & critters, const Tower& rangeOwner) const {
    std::vector<Critter*> candidates;

    for (Critter* critter : critters){
        if(critter != nullptr && critter->isAlive() && rangeOwner.isInRange(critter->getRow(), critter->getColumn())){
            candidates.push_back(critter);
        }
    }

    return targetingStrategy->selectTarget(candidates);
}

// ---- getters ----

int Tower::getRow() const{
    return row;
}

int Tower::getColumn() const {
    return column;
}

int Tower::getDamage() const {
    return damage;
    }

int Tower::getLevel() const {
    return level;
}

int Tower::getCost() const {
    return cost;
}

int Tower::getUpgradeCost() const {
    if (!purchased || sold || level >= maxLevel) {
        return 0;
    }

    return updateCost();
}

int Tower::getMaxLevel() const {
    return maxLevel;
}

int Tower::getRefundValue() const {
    return investments / 2;
}

double Tower::getRange() const {
    return range;
}

double Tower::getFireRate() const {
    return fireRate;
}

bool Tower::isPurchased() const {
    return purchased;
}

bool Tower::isSold() const {
    return sold;
}

double Tower::getCooldown() const {
    return cooldown;
}

// ---- concrete towers: cost, damage, range, fire rate ----

BasicTower::BasicTower(int row, int column)
: Tower(row, column, 100, 20, 3.0, 1.0) {
}

const char* BasicTower::getName() const {
    return "Basic Tower";
}

SplashTower::SplashTower(int row, int column)
: Tower(row, column, 150, 30, 2.0, 0.5), splashRadius(1.5){
}


double SplashTower::getSplashRadius() const {
    return splashRadius;
}

const char* SplashTower::getName() const {
    return "Splash Tower";
}

SlowTower::SlowTower(int row, int column)
: Tower(row, column, 125, 5, 3.0, 1.0), slowFactor(0.5), slowDuration(2.0){
}


double SlowTower::getSlowFactor() const {
    return slowFactor;
}

double SlowTower::getSlowDuration() const {
    return slowDuration;
}

const char* SlowTower::getName() const {
    return "Slow Tower";
}

// one tick of a tower that is not decorated: it is its own attacker
bool Tower::update(double deltaTime, const std::vector<Critter*> & critters){
    return performUpdate(deltaTime, critters, *this);
}

// counts the cooldown down; when it is over, chooses a target in range and shoots it through 'attacker'
bool Tower::performUpdate(double deltaTime, const std::vector<Critter*> & critters, Tower& attacker){
    if(!purchased || sold){
        return false;
    }

    cooldown -= deltaTime;
    if(cooldown > 0.0){
        return false;
    }
    cooldown = 0.0;

    Critter* target = selectTargetFor(critters, attacker);
    if(target == nullptr){
        return false;
    }

    attacker.applyAttack(target, critters); // the tower itself, or the outermost decorator wrapping it
    cooldown = 1.0 / attacker.getFireRate(); // a LevelUpDecorator shoots faster

    notify(); // the tower shot: its cooldown was reset
    return true;
}

// ---- what one shot of each concrete tower does ----

void BasicTower::applyAttack(Critter* target, const std::vector<Critter*> &){
    target->takeDamage(damage);
}

// every critter within the splash radius of the target (the target included) takes the full damage
void SplashTower::applyAttack(Critter* target, const std::vector<Critter*> & critters){
    for(Critter* critter : critters){
        if(critter == nullptr){
            continue;
        }

        double rowDiff = critter->getRow() - target->getRow();
        double columnDiff = critter->getColumn() - target->getColumn();

        if(std::sqrt(rowDiff * rowDiff + columnDiff * columnDiff) <= splashRadius){
            critter->takeDamage(damage);
        }
    }
}

// small damage, then the target is slowed down
void SlowTower::applyAttack(Critter* target, const std::vector<Critter*> &){
    target->takeDamage(damage);
    target->applySlow(slowFactor, slowDuration);
}

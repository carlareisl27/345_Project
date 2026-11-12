#include "Tower.h"
#include "Critter.h"

#include <cmath>
#include <limits>

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
  sold(false){

}

int Tower::updateCost() const{
    return cost*level;
}

bool Tower::purchase(int & coins){
    if(purchased || sold || coins < cost){
        return false;
    }

    coins -= cost;
    purchased = true;

    notify(); // observer pattern: the tower changed, its views are refreshed
    return true;
}

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

bool Tower::isInRange(int targetRow, int targetColumn) const {
    double rowDiff = targetRow - row;
    double columnDiff = targetColumn - column;

    double distance = std::sqrt(rowDiff * rowDiff + columnDiff * columnDiff);

    return distance <= range;
}

Critter* Tower::selectTarget(
    const std::vector<Critter*> & critters) const {
        Critter* closestCritter = nullptr;

        double shortestDist = std::numeric_limits<double>::max();

        for (Critter* critter : critters){
            if(critter == nullptr || !critter->isAlive()){
                continue;
            }

            int targetRow = critter->getRow();
            int targetColumn = critter->getColumn();

                if(!isInRange(targetRow, targetColumn)){
                    continue;
                }

                double rowDiff = targetRow - row;
                double columnDiff = targetColumn - column;

                double distance = std::sqrt(rowDiff * rowDiff + columnDiff * columnDiff);

                if(distance < shortestDist){
                    shortestDist = distance;
                    closestCritter = critter;
                }
    
        }

        return closestCritter;
    }

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

bool Tower::update(double deltaTime, const std::vector<Critter*> & critters){
    if(!purchased || sold){
        return false;
    }

    cooldown -= deltaTime;
    if(cooldown > 0.0){
        return false;
    }
    cooldown = 0.0;

    Critter* target = selectTarget(critters);
    if(target == nullptr){
        return false;
    }

    applyAttack(target, critters);
    cooldown = 1.0 / fireRate;

    notify(); // the tower shot: its cooldown was reset
    return true;
}

void BasicTower::applyAttack(Critter* target, const std::vector<Critter*> &){
    target->takeDamage(damage);
}

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

void SlowTower::applyAttack(Critter* target, const std::vector<Critter*> &){
    target->takeDamage(damage);
    target->applySlow(slowFactor, slowDuration);
}

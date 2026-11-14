// Decorator pattern (Assignment 3, Part 3): one level of a tower. Each level bought during play wraps the tower in one
// more LevelUpDecorator (see levelUp in TowerUpgrades.cpp), which changes the level, stats, next cost and refund.
#include "LevelUpDecorator.h"
#include "Critter.h"

LevelUpDecorator::LevelUpDecorator(Tower* tower, int pricePaid)
: TowerDecorator(tower), pricePaid(pricePaid < 0 ? 0 : pricePaid) {
    notify();
}

int LevelUpDecorator::getLevelsAdded() const {
    return 1;
}

bool LevelUpDecorator::upgrade(int&) {
    return false; // levels are added with levelUp(), never by the wrapped Tower::upgrade
}

int LevelUpDecorator::getLevel() const {
    return wrapped->getLevel() + 1;
}

int LevelUpDecorator::getDamage() const {
    return wrapped->getDamage() * 6 / 5; // x1.2 rounded down, in integers so 20 gives exactly 24
}

double LevelUpDecorator::getRange() const {
    return wrapped->getRange() * 1.2;
}

double LevelUpDecorator::getFireRate() const {
    return wrapped->getFireRate() * 1.1;
}

int LevelUpDecorator::getRefundValue() const {
    return wrapped->getRefundValue() + pricePaid / 2; // half of what this level cost comes back on sale
}

int LevelUpDecorator::getPricePaid() const {
    return pricePaid;
}

const char* LevelUpDecorator::getName() const {
    return TowerDecorator::getName(); // "<base name> Lv<1 + number of level-up decorators>"
}

const char* LevelUpDecorator::getDecoratorLabel() const {
    return "Level Up";
}

void LevelUpDecorator::applyAttack(Critter* target, const std::vector<Critter*> & critters) {
    const int bonus = getDamage() - wrapped->getDamage(); // what this level adds on top of the wrapped tower

    TowerDecorator::applyAttack(target, critters);

    if (bonus > 0 && target->isAlive()) {
        target->takeDamage(bonus);
    }
}

#include "TowerDecorator.h"

#include <cmath>
#include <stdexcept>
#include <vector>

Tower* TowerDecorator::requireTower(Tower* tower) {
    if (tower == nullptr) {
        throw std::invalid_argument("A tower decorator needs a tower to wrap.");
    }
    return tower;
}

// The Tower base gets the wrapped tower's values so the decorator is a valid Tower on its own;
// every getter is still forwarded, so these copies are never what the game reads.
TowerDecorator::TowerDecorator(Tower* tower)
// (the arguments of a call are evaluated in any order, so each one goes through requireTower)
: Tower(requireTower(tower)->getRow(), requireTower(tower)->getColumn(), requireTower(tower)->getCost(),
        requireTower(tower)->getDamage(), requireTower(tower)->getRange(), requireTower(tower)->getFireRate()),
  wrapped(tower) {
}

TowerDecorator::~TowerDecorator() {
    if (wrapped != nullptr) {
        delete wrapped;
    }
}

const Tower* TowerDecorator::getWrappedTower() const {
    return wrapped;
}

int TowerDecorator::getLevelsAdded() const {
    return 0;
}

// ---- forwarded protected operations (TowerDecorator is a friend of Tower) ----

void TowerDecorator::applyAttack(Critter* target, const std::vector<Critter*> & critters) {
    wrapped->applyAttack(target, critters);
}

Critter* TowerDecorator::selectTargetFor(const std::vector<Critter*> & critters, const Tower& rangeOwner) const {
    return wrapped->selectTargetFor(critters, rangeOwner);
}

bool TowerDecorator::performUpdate(double deltaTime, const std::vector<Critter*> & critters, Tower& attacker) {
    return wrapped->performUpdate(deltaTime, critters, attacker);
}

// ---- name ----

const char* TowerDecorator::getName() const {
    int levels = 0;
    std::vector<const char*> labels; // outermost decorator first

    const Tower* current = this;
    const TowerDecorator* decorator = dynamic_cast<const TowerDecorator*>(current);
    while (decorator != nullptr) {
        if (decorator->getLevelsAdded() > 0) {
            levels += decorator->getLevelsAdded();
        } else {
            labels.push_back(decorator->getDecoratorLabel());
        }
        current = decorator->wrapped;
        decorator = dynamic_cast<const TowerDecorator*>(current);
    }

    nameCache = current->getName(); // the concrete tower at the bottom of the chain
    if (levels > 0) {
        nameCache += " Lv" + std::to_string(1 + levels);
    }
    for (auto label = labels.rbegin(); label != labels.rend(); ++label) { // in the order they were applied
        nameCache += " +";
        nameCache += *label;
    }

    return nameCache.c_str();
}

// ---- mutating operations: forward, then tell this decorator's observers ----

bool TowerDecorator::purchase(int& coins) {
    bool result = wrapped->purchase(coins);
    notify();
    return result;
}

bool TowerDecorator::upgrade(int& coins) {
    bool result = wrapped->upgrade(coins);
    notify();
    return result;
}

// the refund is computed here, on the outermost decorator, so it includes what every decorator adds to it
// (LevelUpDecorator gives back half of the price of its level). The wrapped chain is then marked as sold.
int TowerDecorator::sell(int& coins) {
    if (!isPurchased() || isSold()) {
        return 0;
    }

    const int refund = getRefundValue();
    int ignored = 0; // the wrapped tower's own refund is already part of 'refund'
    wrapped->sell(ignored);
    coins += refund;

    notify();
    return refund;
}

bool TowerDecorator::moveTo(int newRow, int newColumn) {
    bool result = wrapped->moveTo(newRow, newColumn);
    notify();
    return result;
}

bool TowerDecorator::update(double deltaTime, const std::vector<Critter*> & critters) {
    // this decorator is the outermost one: it is the attacker, so every decorator of the chain applies its effect
    bool shot = performUpdate(deltaTime, critters, *this);
    notify();
    return shot;
}

void TowerDecorator::setTargetingStrategy(std::unique_ptr<TargetingStrategy> strategy) {
    wrapped->setTargetingStrategy(std::move(strategy));
    notify();
}

const TargetingStrategy& TowerDecorator::getTargetingStrategy() const {
    return wrapped->getTargetingStrategy(); // the strategy lives in the innermost tower
}

bool TowerDecorator::isInRange(int targetRow, int targetColumn) const {
    double rowDiff = targetRow - getRow();
    double columnDiff = targetColumn - getColumn();
    return std::sqrt(rowDiff * rowDiff + columnDiff * columnDiff) <= getRange();
}

// ---- getters ----

// the price of the next level, computed from the decorated level so that every LevelUpDecorator raises it:
// cost x current level (the same rule as Tower::upgrade), 0 if the tower cannot be upgraded now.
int TowerDecorator::getUpgradeCost() const {
    if (!isPurchased() || isSold() || getLevel() >= getMaxLevel()) {
        return 0;
    }
    return getCost() * getLevel();
}

// forwarded

int TowerDecorator::getRow() const { return wrapped->getRow(); }
int TowerDecorator::getColumn() const { return wrapped->getColumn(); }
int TowerDecorator::getDamage() const { return wrapped->getDamage(); }
int TowerDecorator::getLevel() const { return wrapped->getLevel(); }
int TowerDecorator::getCost() const { return wrapped->getCost(); }
int TowerDecorator::getMaxLevel() const { return wrapped->getMaxLevel(); }
int TowerDecorator::getRefundValue() const { return wrapped->getRefundValue(); }
double TowerDecorator::getRange() const { return wrapped->getRange(); }
double TowerDecorator::getFireRate() const { return wrapped->getFireRate(); }
bool TowerDecorator::isPurchased() const { return wrapped->isPurchased(); }
bool TowerDecorator::isSold() const { return wrapped->isSold(); }
double TowerDecorator::getCooldown() const { return wrapped->getCooldown(); }

#include "SplashDecorator.h"
#include "Critter.h"

#include <cmath>

SplashDecorator::SplashDecorator(Tower* tower, double splashRadius)
: TowerDecorator(tower), splashRadius(splashRadius < 0.0 ? 0.0 : splashRadius) {
}

double SplashDecorator::getSplashRadius() const {
    return splashRadius;
}

int SplashDecorator::getSplashDamage() const {
    const int half = wrapped->getDamage() / 2;
    return half < 1 ? 1 : half;
}

const char* SplashDecorator::getDecoratorLabel() const {
    return "Splash";
}

void SplashDecorator::applyAttack(Critter* target, const std::vector<Critter*> & critters) {
    // values taken before the hit: the damage at the time of the attack and the target's cell
    const int splashDamage = getSplashDamage();
    const int targetRow = target->getRow();
    const int targetColumn = target->getColumn();

    TowerDecorator::applyAttack(target, critters);

    // the splash is applied even if the target was killed by the wrapped attack
    for (Critter* critter : critters) {
        if (critter == nullptr || critter == target || !critter->isActive()) {
            continue;
        }

        const double rowDiff = critter->getRow() - targetRow;
        const double columnDiff = critter->getColumn() - targetColumn;

        if (std::sqrt(rowDiff * rowDiff + columnDiff * columnDiff) <= splashRadius) {
            critter->takeDamage(splashDamage);
        }
    }
}

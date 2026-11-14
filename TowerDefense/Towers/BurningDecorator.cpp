// Decorator pattern (Assignment 3, Part 3): the Burning upgrade. A hit sets the target on fire, and the burn deals
// a few more ticks of damage after the hit. The burns are kept per critter in 'burns'.
#include "BurningDecorator.h"
#include "Critter.h"

#include <algorithm>
#include <cmath>

// out-of-range values are corrected instead of refused (see the header)
BurningDecorator::BurningDecorator(Tower* tower, double tickDamagePercent, int tickCount, double tickInterval)
: TowerDecorator(tower),
  tickDamagePercent(tickDamagePercent < 0.0 ? 0.0 : tickDamagePercent),
  tickCount(tickCount < 1 ? 1 : tickCount),
  tickInterval(tickInterval <= 0.0 ? 0.01 : tickInterval) {
}

double BurningDecorator::getTickDamagePercent() const { return tickDamagePercent; }
int BurningDecorator::getTickCount() const { return tickCount; }
double BurningDecorator::getTickInterval() const { return tickInterval; }

const char* BurningDecorator::getDecoratorLabel() const {
    return "Burning";
}

// ---- what is burning now (used by the tests and by the GUI to draw the burning ring) ----

bool BurningDecorator::isBurning(const Critter* critter) const {
    return burns.find(critter) != burns.end();
}

int BurningDecorator::getBurnTicksLeft(const Critter* critter) const {
    auto found = burns.find(critter);
    return found == burns.end() ? 0 : found->second.ticksLeft;
}

int BurningDecorator::getBurnTickDamage(const Critter* critter) const {
    auto found = burns.find(critter);
    return found == burns.end() ? 0 : found->second.tickDamage;
}

int BurningDecorator::getBurningCount() const {
    return static_cast<int>(burns.size());
}

// runs the wrapped attack, then starts a burn on the target, or restarts the one it already has
void BurningDecorator::applyAttack(Critter* target, const std::vector<Critter*> & critters) {
    const int hitDamage = wrapped->getDamage(); // damage of the hit, captured at the moment of the attack

    TowerDecorator::applyAttack(target, critters);

    if (!target->isAlive()) {
        return; // nothing left to burn
    }

    auto found = burns.find(target);
    if (found != burns.end()) {
        // already burning: restart the burn, keep its per-tick damage
        found->second.ticksLeft = tickCount;
        found->second.timeSinceLastTick = 0.0;
        return;
    }

    const int tickDamage = std::max(1, static_cast<int>(std::floor(hitDamage * tickDamagePercent)));
    burns[target] = BurnState{tickDamage, tickCount, 0.0};
}

// every update of the chain first advances the burns, then lets the wrapped tower shoot
bool BurningDecorator::performUpdate(double deltaTime, const std::vector<Critter*> & critters, Tower& attacker) {
    advanceBurns(deltaTime, critters);
    return TowerDecorator::performUpdate(deltaTime, critters, attacker);
}

// deals the ticks that are due and removes the burns that are over, or whose critter is dead or gone
void BurningDecorator::advanceBurns(double deltaTime, const std::vector<Critter*> & critters) {
    for (auto burn = burns.begin(); burn != burns.end();) {
        auto listed = std::find(critters.begin(), critters.end(), burn->first);

        // a critter that is not in the list anymore left the game (dead or escaped): forget it without touching it
        if (listed == critters.end() || !(*listed)->isAlive()) {
            burn = burns.erase(burn);
            continue;
        }

        Critter* critter = *listed;
        BurnState& state = burn->second;
        state.timeSinceLastTick += deltaTime;

        if (state.timeSinceLastTick >= tickInterval) {
            state.timeSinceLastTick -= tickInterval;
            critter->takeDamage(state.tickDamage);
            state.ticksLeft--;
        }

        if (state.ticksLeft <= 0 || !critter->isAlive()) {
            burn = burns.erase(burn);
        } else {
            ++burn;
        }
    }
}

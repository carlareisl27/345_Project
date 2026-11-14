// Part 3 (critters): one critter. It finds the shortest path from the entry to the exit, walks it cell by cell,
// can be damaged and slowed, and pays a reward when killed or steals coins when it escapes.
#include "Critter.h"
#include "Map.h"

#include <algorithm>
#include <cmath>
#include <queue>

// how far a blocked critter stays into its cell (0 to 1), so it is drawn right behind the critter in front.
static const double WAITING_PROGRESS = 0.8;

Critter::Critter(CritterType type, int level)
: type(type), state(CritterState::WAITING),
level(level < 1 ? 1 : level),
maxHitPoints(0), hitPoints(0), strength(0), speed(0.0), reward(0),
slowFactor(1.0), slowTimer(0.0),
pathIndex(0), progress(0.0),
rewardClaimed(false), penaltyClaimed(false) {

    int baseHitPoints = 0;
    int baseStrength = 0;
    double baseSpeed = 0.0;
    int baseReward = 0;

    switch (type) {
        case CritterType::MELEE:
            baseHitPoints = 30; baseStrength = 1; baseSpeed = 1.0; baseReward = 2;
            break;
        case CritterType::CASTER:
            baseHitPoints = 20; baseStrength = 2; baseSpeed = 1.1; baseReward = 3;
            break;
        case CritterType::CANNON:
            baseHitPoints = 60; baseStrength = 3; baseSpeed = 0.8; baseReward = 5;
            break;
        case CritterType::SUPER:
            baseHitPoints = 120; baseStrength = 5; baseSpeed = 0.9; baseReward = 10;
            break;
    }

    maxHitPoints = static_cast<int>(std::round(baseHitPoints * (1.0 + 0.20 * (this->level - 1))));
    hitPoints = maxHitPoints;
    strength = baseStrength + (this->level - 1) / 3;
    speed = baseSpeed * std::min(1.0 + 0.03 * (this->level - 1), 1.5);

    reward = baseReward * this->level;
}

// breadth-first search from the entry: the first time it reaches the exit it has found a shortest path.
bool Critter::findPath(const Map& map) {
    path.clear();

    std::pair<int, int> entry = map.getEntry();
    std::pair<int, int> exit = map.getExit();

    if (!map.isPathCell(entry.first, entry.second) || !map.isPathCell(exit.first, exit.second)) {
        return false;
    }

    int rows = map.getRows();
    int columns = map.getColumns();

    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(columns, false));
    std::vector<std::vector<std::pair<int, int>>> parent(rows, std::vector<std::pair<int, int>>(columns, std::make_pair(-1, -1)));

    std::queue<std::pair<int, int>> positions;
    positions.push(entry);
    visited[entry.first][entry.second] = true;

    const int rowOffSet[] = {-1, 1, 0, 0};
    const int columnOffSet[] = {0, 0, -1, 1};

    bool found = false;
    while (!positions.empty() && !found) {
        std::pair<int, int> current = positions.front();
        positions.pop();

        if (current == exit) {
            found = true;
            break;
        }

        for (int i = 0; i < 4; i++) {
            int newRow = current.first + rowOffSet[i];
            int newColumn = current.second + columnOffSet[i];

            if (map.isPathCell(newRow, newColumn) && !visited[newRow][newColumn]) {
                visited[newRow][newColumn] = true;
                parent[newRow][newColumn] = current;
                positions.push(std::make_pair(newRow, newColumn));
            }
        }
    }

    if (!found) {
        return false;
    }

    for (std::pair<int, int> cell = exit; cell != std::make_pair(-1, -1); cell = parent[cell.first][cell.second]) {
        path.push_back(cell);
    }
    std::reverse(path.begin(), path.end());

    return true;
}

// puts a waiting critter on the entry cell. Refused if it is already out, or if the map has no path to the exit.
bool Critter::spawn(const Map& map) {
    if (state != CritterState::WAITING) {
        return false;
    }

    if (!findPath(map)) {
        return false;
    }

    pathIndex = 0;
    progress = 0.0;
    state = CritterState::ACTIVE;
    notify(); // observer pattern: the critter is now on the map
    return true;
}

// the cell the critter walks into next (the exit cell once it is at the end of its path)
std::pair<int, int> Critter::chooseNextCell() const {
    if (path.empty()) {
        return std::make_pair(-1, -1);
    }

    if (pathIndex + 1 >= static_cast<int>(path.size())) {
        return path.back();
    }

    return path[pathIndex + 1];
}

void Critter::update(double deltaTime, const std::function<bool(int, int)>& isOccupied) {
    if (state != CritterState::ACTIVE || deltaTime <= 0.0) {
        return;
    }

    // values shown by the observers, to tell them only when one of them really changed
    // (moving inside a cell is not a change, entering another cell is).
    const int indexBefore = pathIndex;
    const bool slowedBefore = slowTimer > 0.0;

    double currentSpeed = getCurrentSpeed();

    if (slowTimer > 0.0) {
        slowTimer = std::max(0.0, slowTimer - deltaTime);
    }

    progress += currentSpeed * deltaTime;

    int lastIndex = static_cast<int>(path.size()) - 1;
    while (progress >= 1.0 && pathIndex < lastIndex) {
        const std::pair<int, int>& next = path[pathIndex + 1];
        bool nextIsExit = (pathIndex + 1 == lastIndex);

        // the next cell is taken by another critter: wait behind it instead of sharing the cell.
        if (!nextIsExit && isOccupied && isOccupied(next.first, next.second)) {
            progress = std::min(progress, WAITING_PROGRESS);
            break;
        }

        pathIndex++;
        progress -= 1.0;
    }

    if (pathIndex >= lastIndex) {
        state = CritterState::ESCAPED;
    }

    if (pathIndex != indexBefore || state != CritterState::ACTIVE || slowedBefore != (slowTimer > 0.0)) {
        notify();
    }
}

// removes hit points; returns true only for the hit that kills the critter
bool Critter::takeDamage(int amount) {
    if (state != CritterState::ACTIVE || amount <= 0) {
        return false;
    }

    hitPoints -= amount;

    if (hitPoints <= 0) {
        hitPoints = 0;
        state = CritterState::DEAD;
        notify();
        return true;
    }

    notify();
    return false;
}

// the critter keeps 'factor' of its speed. The strongest slow wins, and the slow lasts at least 'duration' seconds.
void Critter::applySlow(double factor, double duration) {
    if (state != CritterState::ACTIVE || factor <= 0.0 || factor >= 1.0 || duration <= 0.0) {
        return;
    }

    if (slowTimer <= 0.0 || factor < slowFactor) {
        slowFactor = factor;
    }
    slowTimer = std::max(slowTimer, duration);
    notify();
}

// the reward of a dead critter is paid once, the penalty of an escaped critter is taken once
int Critter::claimReward() {
    if (state != CritterState::DEAD || rewardClaimed) {
        return 0;
    }

    rewardClaimed = true;
    return reward;
}

int Critter::claimPenalty() {
    if (state != CritterState::ESCAPED || penaltyClaimed) {
        return 0;
    }

    penaltyClaimed = true;
    return strength;
}

// ---- getters ----

int Critter::getRow() const {
    return path.empty() ? -1 : path[pathIndex].first;
}

int Critter::getColumn() const {
    return path.empty() ? -1 : path[pathIndex].second;
}

bool Critter::isAlive() const {
    return hitPoints > 0;
}

bool Critter::isActive() const {
    return state == CritterState::ACTIVE;
}

bool Critter::hasEscaped() const {
    return state == CritterState::ESCAPED;
}

CritterType Critter::getType() const {
    return type;
}

CritterState Critter::getState() const {
    return state;
}

int Critter::getLevel() const {
    return level;
}

int Critter::getHitPoints() const {
    return hitPoints;
}

int Critter::getMaxHitPoints() const {
    return maxHitPoints;
}

int Critter::getStrength() const {
    return strength;
}

int Critter::getReward() const {
    return reward;
}

double Critter::getSpeed() const {
    return speed;
}

double Critter::getCurrentSpeed() const {
    return slowTimer > 0.0 ? speed * slowFactor : speed;
}

double Critter::getSlowTimeLeft() const {
    return slowTimer;
}

int Critter::getPathIndex() const {
    return path.empty() ? -1 : pathIndex;
}

int Critter::getPathLength() const {
    return static_cast<int>(path.size());
}

std::pair<double, double> Critter::getExactPosition() const {
    if (path.empty()) {
        return std::make_pair(-1.0, -1.0);
    }

    if (pathIndex < 0 || pathIndex >= static_cast<int>(path.size())) {
        return std::make_pair(static_cast<double>(path.back().first), static_cast<double>(path.back().second));
    }

    if (pathIndex + 1 >= static_cast<int>(path.size())) {
        return std::make_pair(static_cast<double>(path[pathIndex].first), static_cast<double>(path[pathIndex].second));
    }

    double clampedProgress = progress;
    if (clampedProgress < 0.0) {
        clampedProgress = 0.0;
    } else if (clampedProgress > 1.0) {
        clampedProgress = 1.0;
    }

    const std::pair<int, int>& current = path[pathIndex];
    const std::pair<int, int>& next = path[pathIndex + 1];

    double row = static_cast<double>(current.first) + (static_cast<double>(next.first) - static_cast<double>(current.first)) * clampedProgress;
    double column = static_cast<double>(current.second) + (static_cast<double>(next.second) - static_cast<double>(current.second)) * clampedProgress;
    return std::make_pair(row, column);
}

const char* Critter::getName() const {
    switch (type) {
        case CritterType::MELEE: return "Melee";
        case CritterType::CASTER: return "Caster";
        case CritterType::CANNON: return "Cannon";
        case CritterType::SUPER: return "Super";
    }
    return "Unknown";
}

char Critter::getSymbol() const {
    switch (type) {
        case CritterType::MELEE: return 'M';
        case CritterType::CASTER: return 'C';
        case CritterType::CANNON: return 'N';
        case CritterType::SUPER: return 'S';
    }
    return '?';
}

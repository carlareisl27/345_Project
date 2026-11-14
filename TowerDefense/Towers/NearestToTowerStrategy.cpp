// Strategy pattern (Assignment 3, Part 1): the default targeting, the candidate closest to the tower.
#include "NearestToTowerStrategy.h"
#include "Tower.h"
#include "Critter.h"

#include <cmath>
#include <limits>

// measures from the tower's current cell, so the strategy stays right after the tower is moved
NearestToTowerStrategy::NearestToTowerStrategy(const Tower* tower)
: tower(tower), fixedRow(0), fixedColumn(0) {
}

// measures from a fixed cell (for testing the strategy without a tower)
NearestToTowerStrategy::NearestToTowerStrategy(int row, int column)
: tower(nullptr), fixedRow(row), fixedColumn(column) {
}

// straight-line (Euclidean) distance in cells, the same measure the tower uses for its range
Critter* NearestToTowerStrategy::selectTarget(const std::vector<Critter*>& candidates) const {
    const int towerRow = tower != nullptr ? tower->getRow() : fixedRow;
    const int towerColumn = tower != nullptr ? tower->getColumn() : fixedColumn;

    Critter* best = nullptr;
    double shortestDistance = std::numeric_limits<double>::max();

    for (Critter* critter : candidates) {
        if (critter == nullptr) {
            continue;
        }

        const double rowDiff = critter->getRow() - towerRow;
        const double columnDiff = critter->getColumn() - towerColumn;
        const double distance = std::sqrt(rowDiff * rowDiff + columnDiff * columnDiff);

        // strict '<': on a tie the first candidate stays selected
        if (distance < shortestDistance) {
            shortestDistance = distance;
            best = critter;
        }
    }

    return best;
}

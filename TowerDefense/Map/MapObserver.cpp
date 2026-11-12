#include "MapObserver.h"
#include "Map.h"

#include <algorithm>
#include <iomanip>
#include <string>

MapObserver::MapObserver(Map* map, std::ostream& out)
: map(map), out(out), refreshCount(0) {
    if (this->map != nullptr) {
        this->map->attach(this); // plugs the view to the map
    }
}

MapObserver::~MapObserver() {
    if (map != nullptr) {
        map->detach(this);
    }
}

void MapObserver::update(Observable* subject) {
    // only the map this view was made for is displayed
    if (subject == map) {
        display();
    }
}

void MapObserver::subjectDestroyed(Observable* subject) {
    if (subject == map) {
        map = nullptr;
    }
}

void MapObserver::display() {
    refreshCount++;

    if (map == nullptr) {
        out << "[Map view] the map no longer exists.\n";
        return;
    }

    const int rows = map->getRows();
    const int columns = map->getColumns();
    const std::pair<int, int> entry = map->getEntry();
    const std::pair<int, int> exit = map->getExit();

    const int width = std::max(5 + 3 * columns, 68); // wide enough for the legend line
    out << std::string(width, '=') << '\n';
    out << " MAP VIEW  (refresh #" << refreshCount << ")  " << rows << " x " << columns << '\n';
    out << std::string(width, '=') << '\n';

    // column numbers
    out << "    ";
    for (int column = 0; column < columns; column++) {
        out << std::setw(3) << column;
    }
    out << '\n';

    // one line per row, same letters as Map::displayMap plus the tower letters
    for (int row = 0; row < rows; row++) {
        out << std::setw(3) << row << " ";
        for (int column = 0; column < columns; column++) {
            char symbol = '#';
            if (entry == std::make_pair(row, column)) {
                symbol = 'E';
            } else if (exit == std::make_pair(row, column)) {
                symbol = 'X';
            } else if (map->hasTower(row, column)) {
                symbol = map->getTowerSymbol(row, column);
            } else if (map->isPathCell(row, column)) {
                symbol = '.';
            }
            out << "  " << symbol;
        }
        out << '\n';
    }

    out << " Legend: E entry, X exit, . path, # scenery, other letters = towers\n";
    out << " Towers on the map: " << map->getTowerCount()
        << "   Valid map: " << (map->isValidMap() ? "yes" : "no") << '\n';
    out << std::string(width, '=') << "\n\n";
}

int MapObserver::getRefreshCount() const {
    return refreshCount;
}

const Map* MapObserver::getMap() const {
    return map;
}

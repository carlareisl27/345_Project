#include "Map.h"

#include <fstream> // library used to save the map to a text file and load it back
#include <iostream>
#include <queue> // library used to implement the breadth-first search
#include <stdexcept> // library used to provide an invalid argument exception 

/*
Constructor:
Creates a map with the number of rows and columns specified by the user. It initializes every cell as SCENERY
The entry and exit points are unassigned
*/
Map::Map(int rows, int columns) 
: rows(rows), columns(columns), // initializes maps dimensions
grid(rows > 0 ? rows : 0, std::vector<CellType>(columns > 0 ? columns : 0, CellType::SCENERY)), // initializes the grid with SCENERY cells and creates the two dimesional grid.
entry(-1, -1), exit(-1, -1), // initializes the entry and exit coordinattes to (-1, -1), these are outside the map and are used as placeholders.
entrySet(false), exitSet(false), // initializes entry and exit points as unassigned.
towers(rows > 0 ? rows : 0, std::vector<char>(columns > 0 ? columns : 0, '\0')) { // no towers at the start.
    // Indicates that the map dimensions must be positive and throws an exception if they are not.
    if (rows <= 0 || columns <= 0) {
        throw std::invalid_argument("Rows and columns must be positive integers.");
    }
}

// function isInsideMap determines whether the row and column given by the user are inside the map.
bool Map::isInsideMap(int row, int column) const {
    return row >= 0 && row < rows && column >= 0 && column < columns;
}

// function setCell allows for a cell to be changed between PATH and SCENERY
bool Map::setCell(int row, int column, CellType type){ 
    // checks if the coordinates are valid
    if (!isInsideMap(row, column)) {
        return false;
    }

    // prevents for entry and exit cells to be changed from PATH
    if (entrySet && entry == std::make_pair(row, column)){
        return type == CellType::PATH;
    }

    if (exitSet && exit == std::make_pair(row, column)){
        return type == CellType::PATH;
    }

    // a tower stands on this cell: it has to stay SCENERY
    if (hasTower(row, column)) {
        return type == CellType::SCENERY;
    }

    // same type as before: nothing changes, so the observers are not told anything
    if (grid[row][column] == type) {
        return true;
    }
    
    // updates the cell is valid
    grid[row][column] = type;
    notify(); // observer pattern: the map changed, its views are refreshed
    return true;
}

// function setEntry assigns the entry point given the coordinates of the user.
bool Map::setEntry(int row, int column) {
    // checks if coordinates are inside the map
    if (!isInsideMap(row, column)) {
        return false;
    }

    // avoids for the entry and exit points to be on the same cell
    if (exitSet && exit == std::make_pair(row, column)) {
        return false; // Entry and exit cannot be the same cell
    }

    // the entry is a PATH cell, it cannot be under a tower
    if (hasTower(row, column)) {
        return false;
    }

    // stores the entry coordinates
    entry = std::make_pair(row, column);
    entrySet = true;
    
    // Ensures entry is always a PATH
    grid [row][column] = CellType::PATH;

    notify();
    return true;
}

// function setExit assigns the exit point given the coordinates of the user.
bool Map::setExit(int row, int column){
    // checks if coordinates are inside the map
    if(!isInsideMap(row, column)){
        return false;
    }

    // avoids for the entry and exit points to be on the same cell
    if (entrySet && entry == std::make_pair(row, column)) {
        return false;
    }

    // the exit is a PATH cell, it cannot be under a tower
    if (hasTower(row, column)) {
        return false;
    }

    // stores the exit coordinates
    exit = std::make_pair(row, column);
    exitSet = true;

    // Ensures exit is always a PATH
    grid[row][column] = CellType::PATH;

    notify();
    return true;
}

// function displayMap prints the map into the console row by row
void Map::displayMap() const {
    // for loop through every row
    for(int row = 0; row < rows; row++){
        // for loop to traverse through every column in a row
        for(int column = 0; column < columns; column++){
            // if the cell is the entry print an E
            if(entrySet && entry == std::make_pair(row, column)){
                std::cout << "E ";
            }
            // if the cell is the exit print a X
            else if(exitSet && exit == std::make_pair(row, column)){
                std::cout << "X ";
            }
            // if a tower stands on the cell print its letter
            else if (hasTower(row, column)){
                std::cout << towers[row][column] << ' ';
            }
            // if the cell is a PATH print a dot
            else if (grid [row][column] == CellType::PATH){
                std::cout << ". ";
            }
            // if the cell is a SCENERY print a #
            else{
                std::cout << "# ";
            }
        
        }

        // move to the next row after finishing with all the columns
        std::cout << '\n';
    }
}

/* function isValidMap will check if the map is valide by checking if the map:
- has an entry point
- has an exit point
- has different entry and exit points
- has coordinates inside the map
- has path cells at entry and exit
- has a connected path from entry to exit
*/
bool Map::isValidMap() const {
    // checks for entry and exit points
    if(!entrySet || !exitSet){
        return false;
    }

    // checks if both points are inside the map
    if(!isInsideMap(entry.first, entry.second) || !isInsideMap(exit.first, exit.second)){
        return false;
    }

    // checks if entry and exit points are not the same, if they are the map is invalid
    if(entry == exit){
        return false;
    }

    // checks if both entry and exit are PATH cells
    if(grid[entry.first][entry.second] != CellType::PATH || grid[exit.first][exit.second] != CellType::PATH){
        return false;
    }

    return hasConnectedPath();
}

// function hasConnectedPath checks if the entry point reaches the exit point by moving through the cells
// it uses a Breadth-first-search by moving through cells until it reaches the exit or every cell has be visited.
bool Map::hasConnectedPath() const {
    // checks for entry and exit points
    if(!entrySet || !exitSet){
        return false;
    }

    // another map is created to keep track of the cells that were already visited, every cell that has not been
    // visited yet shows as false.
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(columns, false));

    // a queue is also created to store the positions of the cells that have not been visited yet
    std::queue<std::pair<int, int>> positions;
    // starts traversal at entry point
    positions.push(entry);
    // marks entry as visited and moves forward
    visited[entry.first][entry.second] = true;

    // it only allows for horizontal and vertical movement and lets only to move up (0), down(1), left(2) and right(3).
    const int rowOffSet[] = {-1, 1, 0, 0};
    const int columnOffSet[] = {0, 0, -1, 1};

    // while loop that will allow the search to continue as long as there are positions in the queue
    while (!positions.empty()){
        auto current = positions.front(); // starts at the front
        positions.pop(); // removes from queue

        // if the position is the exit then a PATH has been found.
        if(current == exit){
            return true;
        }

        // traverses through cells
        for (int i = 0; i < 4; i++){
            int newRow = current.first + rowOffSet[i]; // calculates row of neighbor cell
            int newColumn = current.second + columnOffSet[i]; // calculates column of neighbor cell

            // allows for a neighbor cell to be explored only if it is inside the map, if it has not been explored yet and if it is not a PATH cell 
            if (isInsideMap(newRow, newColumn) && !visited[newRow][newColumn] && grid[newRow][newColumn] == CellType::PATH){
                visited[newRow][newColumn] = true; // marks the neighbor cell as visited
                positions.push(std::make_pair(newRow, newColumn)); // adds cell to the queue
            }
        }
    }
    // if the queue gets empty and it has not reached the exit then there is no connected path from the entry point to the exit point.
    return false;
}

// accessors added for Part 3, they only read the map and never change it.
int Map::getRows() const {
    return rows;
}

int Map::getColumns() const {
    return columns;
}

std::pair<int, int> Map::getEntry() const {
    return entry;
}

std::pair<int, int> Map::getExit() const {
    return exit;
}

// function isPathCell is used by critters to know where they are allowed to walk
bool Map::isPathCell(int row, int column) const {
    return isInsideMap(row, column) && grid[row][column] == CellType::PATH;
}

// function isSceneryCell is used to check that a tower is not placed on the path
bool Map::isSceneryCell(int row, int column) const {
    return isInsideMap(row, column) && grid[row][column] == CellType::SCENERY;
}

// function saveToFile writes the map to a text file.
// first line: rows and columns, then one line per row with E (entry), X (exit), . (path) or # (scenery).
bool Map::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);

    // the file could not be opened (for example the folder is read-only)
    if (!file) {
        return false;
    }

    file << rows << " " << columns << '\n';

    for (int row = 0; row < rows; row++) {
        for (int column = 0; column < columns; column++) {
            if (entrySet && entry == std::make_pair(row, column)) {
                file << 'E';
            }
            else if (exitSet && exit == std::make_pair(row, column)) {
                file << 'X';
            }
            else if (grid[row][column] == CellType::PATH) {
                file << '.';
            }
            else {
                file << '#';
            }
        }
        file << '\n';
    }

    return true;
}

// function loadFromFile reads a map written by saveToFile.
// The new map is built in a separate Map object first, so if the file is bad the current map is not changed.
bool Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);

    // the file does not exist
    if (!file) {
        return false;
    }

    int fileRows = 0;
    int fileColumns = 0;
    file >> fileRows >> fileColumns;

    // the first line must contain two positive numbers
    if (!file || fileRows <= 0 || fileColumns <= 0) {
        return false;
    }

    Map loaded(fileRows, fileColumns);

    for (int row = 0; row < fileRows; row++) {
        std::string line;
        file >> line;

        // every row must have exactly one letter per column
        if (!file || static_cast<int>(line.size()) != fileColumns) {
            return false;
        }

        for (int column = 0; column < fileColumns; column++) {
            char letter = line[column];

            if (letter == 'E') {
                loaded.setEntry(row, column);
            }
            else if (letter == 'X') {
                loaded.setExit(row, column);
            }
            else if (letter == '.') {
                loaded.setCell(row, column, CellType::PATH);
            }
            else if (letter == '#') {
                loaded.setCell(row, column, CellType::SCENERY);
            }
            else {
                return false; // unknown letter in the file
            }
        }
    }

    // everything was read correctly, replace the current map with the loaded one.
    // The assignment keeps this map's observers (see Observable), and they are told about the new map once.
    *this = loaded;
    notify();
    return true;
}

// function placeTower puts a tower on a free SCENERY cell (towers never stand on the path).
bool Map::placeTower(int row, int column, char symbol) {
    if (!isSceneryCell(row, column) || hasTower(row, column)) {
        return false;
    }

    towers[row][column] = (symbol == '\0') ? 'T' : symbol;
    notify();
    return true;
}

// function removeTower takes the tower away (for example when it is sold).
bool Map::removeTower(int row, int column) {
    if (!hasTower(row, column)) {
        return false;
    }

    towers[row][column] = '\0';
    notify();
    return true;
}

// function moveTower moves a tower to another free SCENERY cell. The observers are told once, after the move.
bool Map::moveTower(int fromRow, int fromColumn, int toRow, int toColumn) {
    if (!hasTower(fromRow, fromColumn)) {
        return false;
    }

    // moving a tower onto its own cell changes nothing
    if (fromRow == toRow && fromColumn == toColumn) {
        return true;
    }

    if (!isSceneryCell(toRow, toColumn) || hasTower(toRow, toColumn)) {
        return false;
    }

    towers[toRow][toColumn] = towers[fromRow][fromColumn];
    towers[fromRow][fromColumn] = '\0';
    notify();
    return true;
}

bool Map::hasTower(int row, int column) const {
    return isInsideMap(row, column) && towers[row][column] != '\0';
}

char Map::getTowerSymbol(int row, int column) const {
    return isInsideMap(row, column) ? towers[row][column] : '\0';
}

int Map::getTowerCount() const {
    int count = 0;
    for (const std::vector<char>& line : towers) {
        for (char symbol : line) {
            if (symbol != '\0') {
                count++;
            }
        }
    }
    return count;
}

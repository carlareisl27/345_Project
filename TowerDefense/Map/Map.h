#ifndef MAP_H
#define MAP_H

#include <string> // use of string library for the file names used to save and load maps.
#include <vector> // use of vector library in order to creat a 2D frid for the map.
#include <utility> // use of utility library in order to use the std::pair which will allow to store 
// the entry and exit coordinates for the rows and columns.

#include "Observer.h" // observer pattern (Part 2 of the observer assignment): Map is an Observable, MapObserver displays it.

// class CellType will be used to define the type of cells in the map.
enum class CellType {
    PATH, // critters will move through this cell.
    SCENERY, // towers will be placed on this cell but critter will not be able to move through it.
};

// class Map will be used to create the map itself and will contain all the necessary methods
// to set the entries, exits, and cells. It will also allow the map to be displayed and validated.
// Map is Observable: every successful change (cells, entry, exit, towers placed / moved / removed, loading a file)
// calls notify(), so the attached observers (MapObserver) re-display the map.
class Map : public Observable {
    private: // avoids memebers to be accessed from outside the class.
        int rows; // stores number of rows.
        int columns; // stores number of columns.

        //2D vector that will store the type of each cell in the map.
        std::vector<std::vector<CellType>> grid;

        std::pair<int, int> entry; // coordinates of th entry cell.
        std::pair<int, int> exit; // coordinates of the exit cell.

        // checks if the entry and exit points have been assigned.
        bool entrySet;
        bool exitSet;

        // towers standing on the map: one letter per cell, '\0' when the cell has no tower.
        // The map only stores where towers are (and the letter used to draw them), the Tower objects are owned by the game.
        std::vector<std::vector<char>> towers;

    public: // allows memebers to be accessed from outside the class.
        Map(int rows, int columns); // creates a map with the number of rows and columns specified by the user.

        bool setCell(int row, int column, CellType type); // sets the type of cell to either PATH ot SCENERY.
        bool setEntry(int row, int column); // sets the entry point using the given row and column coordinates.
        bool setExit(int row, int column); // sets the exit point using the given row and column coordinates.
        void displayMap() const; // displays the map in the console.
        bool isValidMap() const; // validates the map by checking if the entry and exit points are set.
        bool isInsideMap(int row, int column) const; // checks if the coordinates given by the user are inside the map boundaries. 
        bool hasConnectedPath() const; // checks if there is a connection between the entry and exit points entered by the user.

        // read-only accessors added for Part 3 (critters need to know the map layout and where the entry and exit are).
        int getRows() const; // returns the number of rows.
        int getColumns() const; // returns the number of columns.
        std::pair<int, int> getEntry() const; // returns the coordinates of the entry cell, (-1, -1) if it is not set.
        std::pair<int, int> getExit() const; // returns the coordinates of the exit cell, (-1, -1) if it is not set.
        bool isPathCell(int row, int column) const; // true only if the coordinates are inside the map and the cell is a PATH.
        bool isSceneryCell(int row, int column) const; // true only if the coordinates are inside the map and the cell is SCENERY (where towers can be placed).

        // saving and loading maps, so a map made in the editor can be chosen later to play on.
        // The file is plain text: the first line is "rows columns", then one line per row using
        // E = entry, X = exit, . = path and # = scenery (the same letters displayMap prints).
        bool saveToFile(const std::string& filename) const; // returns false if the file could not be written.
        bool loadFromFile(const std::string& filename); // returns false (and keeps the old map) if the file is missing or badly written.

        // towers on the map (added for the map observer). A tower can only stand on a SCENERY cell with no other tower,
        // and a cell with a tower cannot be turned into PATH, an entry or an exit.
        bool placeTower(int row, int column, char symbol = 'T'); // false if the cell is not free scenery.
        bool removeTower(int row, int column); // false if there is no tower on that cell.
        bool moveTower(int fromRow, int fromColumn, int toRow, int toColumn); // false (nothing moves) if there is no tower to move or the new cell is not free scenery.
        bool hasTower(int row, int column) const;
        char getTowerSymbol(int row, int column) const; // '\0' when there is no tower on the cell.
        int getTowerCount() const;
};

#endif

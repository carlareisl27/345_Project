#include "Map.h"

#include <iostream>

int main(){
    // creates a 5x5 map
    Map map(5, 5);

    // sets entry and exit points
    map.setEntry(0, 0); // row 0, columns 0
    map.setExit(3, 4); // row 3, column 4

    // path creations, changes each cell from SCENERY to PATH
    map.setCell(0, 1, CellType::PATH);
    map.setCell(0, 2, CellType::PATH);
    map.setCell(1, 2, CellType::PATH);
    map.setCell(2, 2, CellType::PATH);
    map.setCell(2, 3, CellType::PATH);
    map.setCell(2, 4, CellType::PATH);
    map.setCell(2, 4, CellType::PATH);
    map.setCell(3, 4, CellType::PATH);

    // prints map
    std::cout << " Tower Defense Map:\n";
    map.displayMap();

    std::cout << "Validating map....\n";

    // checks is the given path fullfills the requirements 
    if(map.isValidMap()){
        std::cout << "The map is valid.\n";
    } else {
        std::cout << "The map is invalid.\n";
    }

    return 0;
}
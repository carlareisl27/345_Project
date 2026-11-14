// Tests of Part 1 (map): creation, cells, entry and exit, validity of the path, saving and loading a map file.
#include "Map.h"
#include "TestHelper.h"

#include <cstdio> // std::remove, used to delete the test file
#include <stdexcept>

// a 5 x 5 map with a path from the entry (0, 0) to the exit (3, 4)
static Map makeValidMap() {
    Map map(5, 5);
    map.setEntry(0, 0);
    map.setExit(3, 4);
    map.setCell(0, 1, CellType::PATH);
    map.setCell(0, 2, CellType::PATH);
    map.setCell(1, 2, CellType::PATH);
    map.setCell(2, 2, CellType::PATH);
    map.setCell(2, 3, CellType::PATH);
    map.setCell(2, 4, CellType::PATH);
    map.setCell(3, 4, CellType::PATH);
    return map;
}

int main() {
    std::cout << "Part 1 tests (map)\n\n";

    std::cout << "Creation\n";
    bool threw = false;
    try { Map bad(0, 5); } catch (const std::invalid_argument&) { threw = true; }
    check(threw, "a map needs positive dimensions");
    Map blank(3, 7);
    check(blank.getRows() == 3 && blank.getColumns() == 7, "a blank map has the requested size");
    check(blank.isSceneryCell(2, 6) && !blank.isPathCell(2, 6), "a blank map is all scenery");
    check(!blank.isValidMap(), "a blank map is not valid (no entry and exit)");

    std::cout << "Setting cells\n";
    check(blank.setCell(1, 1, CellType::PATH) && blank.isPathCell(1, 1), "a cell can be changed to PATH");
    check(blank.setCell(1, 1, CellType::SCENERY) && blank.isSceneryCell(1, 1), "a cell can be changed back to SCENERY");
    check(!blank.setCell(3, 0, CellType::PATH) && !blank.setCell(0, -1, CellType::PATH), "cells outside the map are refused");
    check(!blank.isPathCell(-1, 0) && !blank.isSceneryCell(0, 7), "cells outside the map are neither path nor scenery");

    std::cout << "Entry and exit\n";
    check(blank.setEntry(0, 0) && blank.isPathCell(0, 0), "the entry becomes a PATH cell");
    check(blank.setExit(2, 6) && blank.isPathCell(2, 6), "the exit becomes a PATH cell");
    check(blank.getEntry() == std::make_pair(0, 0) && blank.getExit() == std::make_pair(2, 6), "entry and exit are stored");
    check(!blank.setExit(0, 0) && !blank.setEntry(2, 6), "entry and exit cannot be on the same cell");
    check(!blank.setEntry(5, 5) && !blank.setExit(-1, 0), "entry and exit must be inside the map");
    check(!blank.setCell(0, 0, CellType::SCENERY), "the entry cannot be turned into scenery");

    std::cout << "Validation\n";
    check(!blank.isValidMap() && !blank.hasConnectedPath(), "entry and exit without a connecting path is invalid");
    Map valid = makeValidMap();
    check(valid.isValidMap() && valid.hasConnectedPath(), "a connected path from entry to exit is valid");

    Map noExit(4, 4);
    noExit.setEntry(0, 0);
    check(!noExit.isValidMap(), "a map without an exit is invalid");

    Map gap = makeValidMap();
    gap.setCell(1, 2, CellType::SCENERY);
    check(!gap.isValidMap(), "a gap in the path makes the map invalid");

    Map diagonal(3, 3);
    diagonal.setEntry(0, 0);
    diagonal.setExit(2, 2);
    diagonal.setCell(1, 1, CellType::PATH);
    check(!diagonal.isValidMap(), "path cells touching only by a corner are not connected");

    Map detour(3, 5);
    detour.setEntry(0, 0);
    detour.setExit(0, 4);
    for (int column = 1; column < 4; column++) detour.setCell(1, column, CellType::PATH);
    detour.setCell(1, 0, CellType::PATH);
    detour.setCell(1, 4, CellType::PATH);
    check(detour.isValidMap(), "a path that goes around through other rows is valid");

    std::cout << "Saving and loading\n";
    Map original = makeValidMap();
    check(original.saveToFile("test_map.txt"), "a map can be saved to a file");
    Map copy(2, 2);
    check(copy.loadFromFile("test_map.txt"), "a saved map can be loaded back");
    check(copy.getRows() == 5 && copy.getColumns() == 5, "the loaded map has the saved size");
    check(copy.getEntry() == std::make_pair(0, 0) && copy.getExit() == std::make_pair(3, 4), "the loaded map has the saved entry and exit");
    check(copy.isPathCell(1, 2) && copy.isSceneryCell(4, 4), "the loaded map has the saved path and scenery cells");
    check(copy.isValidMap(), "the loaded map is still valid");
    std::remove("test_map.txt");

    Map unchanged(3, 3);
    check(!unchanged.loadFromFile("this_file_does_not_exist.txt"), "loading a missing file fails");
    check(unchanged.getRows() == 3 && unchanged.getColumns() == 3, "a failed load keeps the old map");

    return testSummary();
}

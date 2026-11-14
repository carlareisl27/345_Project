#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Map.h"
#include "Tower.h"
#include "Critter.h"
#include "CritterGroup.h"
#include "CritterGroupGenerator.h"
#include "StandardCritterFactory.h"
// Assignment 3: targeting strategies (Strategy) and tower upgrades (Decorator) controlled from the selected-tower panel
#include "NearestToTowerStrategy.h"
#include "NearestToExitStrategy.h"
#include "StrongestCritterStrategy.h"
#include "WeakestCritterStrategy.h"
#include "TowerUpgrades.h"
#include "SplashDecorator.h"
#include "BurningDecorator.h"
#include "FreezingDecorator.h"

#include <windows.h>
#include <windowsx.h>
#include <wincodec.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

// Size of the area the game is designed for. Every position below (map, panel, buttons, texts) is given inside this
// area. The real window can have any size: the whole area is scaled by one factor and centered (see updateView).
constexpr int WINDOW_WIDTH = 1100;
constexpr int WINDOW_HEIGHT = 720;
constexpr int MIN_WINDOW_WIDTH = 640;  // smallest window the player can drag to
constexpr int MIN_WINDOW_HEIGHT = 440;
constexpr int MAP_AREA_LEFT = 24;      // the map grid is centered in a square of this size
constexpr int MAP_AREA_TOP = 76;
constexpr int MAP_AREA_SIZE = 620;
constexpr int PANEL_LEFT = 676;        // side panel (buttons and information)
constexpr int PANEL_TOP = 64;
constexpr int PANEL_WIDTH = 412;
constexpr int PANEL_HEIGHT = 644;
constexpr int PANEL_PADDING = 16;
constexpr int PANEL_CONTENT_LEFT = PANEL_LEFT + PANEL_PADDING;
constexpr int PANEL_CONTENT_WIDTH = PANEL_WIDTH - 2 * PANEL_PADDING;
constexpr int TILES_TOP = PANEL_TOP + 52;  // row of stat tiles under the panel heading
constexpr int TILES_HEIGHT = 52;
constexpr int TILE_GAP = 8;
constexpr int MESSAGE_LEFT = 340;      // status message pill in the header
constexpr int MESSAGE_TOP = 14;
constexpr int MESSAGE_WIDTH = 748;
constexpr int MESSAGE_HEIGHT = 32;
constexpr int BUTTON_LEFT = PANEL_CONTENT_LEFT;  // button column (left half of the panel)
constexpr int BUTTON_TOP = TILES_TOP + TILES_HEIGHT + 12;
constexpr int BUTTON_WIDTH = 172;
constexpr int BUTTON_HEIGHT = 30;
constexpr int BUTTON_GAP = 6;
constexpr int SECTION_LABEL_HEIGHT = 24;
constexpr int SECTION_GAP = 8;
constexpr int INFO_LEFT = BUTTON_LEFT + BUTTON_WIDTH + 18; // text column (right half of the panel)
constexpr int INFO_WIDTH = PANEL_CONTENT_LEFT + PANEL_CONTENT_WIDTH - INFO_LEFT;
constexpr int INFO_LINE = 20;
constexpr int LOG_LINE = 18;
constexpr double FIXED_STEP = 0.05;
constexpr double MAX_FRAME_DELTA = 0.25;
constexpr int MIN_DIMENSION = 3;
constexpr int MAX_DIMENSION = 40; // rows and columns are chosen separately, so 5 x 30 or 10 x 10 are both possible
constexpr int STARTING_COINS = 200;
constexpr int STARTING_LIVES = 10; // the game ends when this many critters reach the exit
constexpr int MAP_SLOTS = 3;       // number of map files the player can save to (map1.txt, map2.txt, map3.txt)
constexpr int DEMO_WAVES = 10;

// Colors. One dark navy theme for the whole window; the map keeps natural grass and sand colors.
constexpr int CORNER_RADIUS = 1; // every rounded rectangle uses this radius
constexpr COLORREF NO_COLOR = 0xFFFFFFFF; // "no fill" / "no border" for the drawing helpers
constexpr COLORREF THEME_BACKGROUND = RGB(13, 17, 26);
constexpr COLORREF THEME_SURFACE = RGB(22, 28, 42);        // panel and map card
constexpr COLORREF THEME_SURFACE_BORDER = RGB(38, 47, 66);
constexpr COLORREF THEME_TILE = RGB(30, 38, 56);
constexpr COLORREF THEME_DIVIDER = RGB(38, 47, 66);
constexpr COLORREF THEME_TEXT = RGB(232, 236, 244);
constexpr COLORREF THEME_TEXT_MUTED = RGB(146, 157, 180);
constexpr COLORREF THEME_TEXT_FAINT = RGB(104, 116, 140);
constexpr COLORREF THEME_TEXT_DISABLED = RGB(80, 92, 116);
constexpr COLORREF THEME_ACCENT = RGB(255, 138, 36);       // the only accent color (orange)
constexpr COLORREF THEME_ACCENT_DARK = RGB(204, 98, 14);   // filled accent buttons
constexpr COLORREF THEME_SUCCESS = RGB(64, 196, 128);
constexpr COLORREF THEME_DANGER = RGB(240, 96, 96);
constexpr COLORREF THEME_GOLD = RGB(245, 196, 80);
constexpr COLORREF THEME_BUTTON = RGB(37, 46, 68);
constexpr COLORREF THEME_BUTTON_HOVER = RGB(50, 62, 90);
constexpr COLORREF THEME_BUTTON_BORDER = RGB(55, 67, 94);
constexpr COLORREF THEME_BUTTON_DISABLED = RGB(28, 34, 50);
constexpr COLORREF THEME_GRASS_LIGHT = RGB(86, 150, 92);
constexpr COLORREF THEME_GRASS_DARK = RGB(78, 140, 85);
constexpr COLORREF THEME_GRASS_LINE = RGB(70, 128, 77);
constexpr COLORREF THEME_PATH = RGB(222, 197, 148);
constexpr COLORREF THEME_MAP_FRAME = RGB(10, 13, 20);
constexpr COLORREF THEME_ENTRY = RGB(46, 170, 104);
constexpr COLORREF THEME_EXIT = RGB(222, 74, 74);
constexpr COLORREF THEME_TOWER_BASIC = RGB(88, 140, 255);
constexpr COLORREF THEME_TOWER_SPLASH = RGB(255, 150, 64);
constexpr COLORREF THEME_TOWER_SLOW = RGB(80, 214, 230);
constexpr COLORREF THEME_TOWER_BASE = RGB(30, 36, 52);
constexpr COLORREF THEME_BURNING = RGB(255, 112, 40); // ring around a burning critter
constexpr COLORREF THEME_TOWER_SHADOW = RGB(44, 92, 54);
constexpr COLORREF THEME_HEALTH_HIGH = RGB(96, 214, 120);

struct Layout {
    double cellSize = 1.0;
    int mapLeft = MAP_AREA_LEFT;
    int mapTop = MAP_AREA_TOP;
    int mapWidth = MAP_AREA_SIZE;
    int mapHeight = MAP_AREA_SIZE;
};

enum class Mode {
    EDIT,
    PLAY,
};

enum class EditTool {
    PATH,
    SCENERY,
    ENTRY,
    EXIT,
};

enum class ShopItem {
    NONE,
    BASIC,
    SPLASH,
    SLOW,
};

enum class ButtonStyle {
    NORMAL,
    PRIMARY, // the main action (Play, Start wave), drawn in the accent color
    DANGER,  // Sell, drawn in red
};

struct Button {
    RECT rect{};
    std::string label;
    std::function<void()> action;
    bool enabled = true;
    bool selected = false;
    bool isHeader = false;            // a section heading drawn between buttons, not clickable
    ButtonStyle style = ButtonStyle::NORMAL;
    COLORREF swatch = NO_COLOR;       // optional colored dot before the label (shop buttons)
    std::string badge;                // optional text on the right (price, file name, ...)
    COLORREF badgeColor = THEME_TEXT;
};

struct ShotEffect {
    POINT from{};
    POINT to{};
    bool splash = false;
    COLORREF color = RGB(255, 255, 255); // color of the tower that fired
    double timeLeft = 0.15;
};

struct GameState {
    Map map{7, 7};
    StandardCritterFactory waveFactory; // Factory pattern: creates the critters of each wave

    Mode mode = Mode::EDIT;
    EditTool editTool = EditTool::PATH;
    ShopItem shopItem = ShopItem::NONE;

    std::vector<std::unique_ptr<Tower>> towers;
    std::unique_ptr<CritterGroup> currentGroup;

    int coins = STARTING_COINS;
    int lives = STARTING_LIVES;
    bool gameOver = false;
    int mapSlot = 1; // which map file Save/Load uses
    int wavesCompleted = 0;
    int selectedTowerIndex = -1;
    int selectedCritterIndex = -1; // index in currentGroup->getCritters(), -1 if no critter is selected
    int sizeStep = 1; // how many rows/columns the Rows and Cols buttons add or remove (1, 5 or 10)
    bool paused = false;
    double speedMultiplier = 1.0;
    double accumulator = 0.0;
    bool demo = false;
    bool draggingPaint = false;
    std::pair<int, int> lastPaintedCell = std::make_pair(-1, -1);
    std::vector<ShotEffect> shots;
    std::vector<std::string> waveLog;
    std::string statusMessage = "Edit the map, then press Play >>.";
    COLORREF statusColor = RGB(255, 255, 255);
    POINT mousePos{0, 0}; // in design coordinates (already converted from window pixels)
    HWND hwnd = nullptr;

    // how the design area is placed in the window, set by updateView
    double viewScale = 1.0;
    int viewOffsetX = 0;
    int viewOffsetY = 0;
};

HFONT g_guiFont = nullptr;

Layout computeLayout(const Map& map) {
    Layout layout;
    const int rows = std::max(1, map.getRows());
    const int columns = std::max(1, map.getColumns());
    const double size = std::min(static_cast<double>(MAP_AREA_SIZE) / columns, static_cast<double>(MAP_AREA_SIZE) / rows);
    layout.cellSize = std::min(size, 90.0);
    layout.mapWidth = static_cast<int>(std::lround(layout.cellSize * static_cast<double>(columns)));
    layout.mapHeight = static_cast<int>(std::lround(layout.cellSize * static_cast<double>(rows)));
    layout.mapLeft = MAP_AREA_LEFT + (MAP_AREA_SIZE - layout.mapWidth) / 2;
    layout.mapTop = MAP_AREA_TOP + (MAP_AREA_SIZE - layout.mapHeight) / 2;
    return layout;
}

POINT cellCenter(const Layout& layout, double row, double column) {
    POINT p{};
    p.x = layout.mapLeft + static_cast<int>(std::lround((column + 0.5) * layout.cellSize));
    p.y = layout.mapTop + static_cast<int>(std::lround((row + 0.5) * layout.cellSize));
    return p;
}

RECT makeRect(int left, int top, int width, int height) {
    RECT rect{};
    rect.left = left;
    rect.top = top;
    rect.right = left + width;
    rect.bottom = top + height;
    return rect;
}

RECT insetRect(const RECT& rect, int dx, int dy) {
    RECT out = rect;
    out.left += dx;
    out.right -= dx;
    out.top += dy;
    out.bottom -= dy;
    return out;
}

std::string toStringFixed(double value, int precision = 1) {
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(precision);
    out << value;
    return out.str();
}

void setStatus(GameState& state, const std::string& message, COLORREF color = RGB(255, 255, 255)) {
    state.statusMessage = message;
    state.statusColor = color;
}

Map makeSampleMap() {
    Map map(7, 7);
    map.setEntry(0, 0);
    map.setExit(6, 5);
    for (int column = 1; column <= 5; ++column) {
        map.setCell(0, column, CellType::PATH);
        map.setCell(2, column, CellType::PATH);
        map.setCell(4, column, CellType::PATH);
    }
    map.setCell(1, 5, CellType::PATH);
    map.setCell(3, 1, CellType::PATH);
    map.setCell(5, 5, CellType::PATH);
    return map;
}

bool hasTowerAt(const GameState& state, int row, int column) {
    for (const auto& tower : state.towers) {
        if (tower->isPurchased() && !tower->isSold() && tower->getRow() == row && tower->getColumn() == column) {
            return true;
        }
    }
    return false;
}

bool isInsideMapCell(const Layout& layout, POINT mouse, int rows, int columns, int& row, int& column) {
    const double x = static_cast<double>(mouse.x - layout.mapLeft);
    const double y = static_cast<double>(mouse.y - layout.mapTop);
    if (x < 0.0 || y < 0.0) {
        return false;
    }

    column = static_cast<int>(x / layout.cellSize);
    row = static_cast<int>(y / layout.cellSize);
    return row >= 0 && row < rows && column >= 0 && column < columns;
}

bool pointInRect(POINT p, const RECT& rect) {
    return p.x >= rect.left && p.x < rect.right && p.y >= rect.top && p.y < rect.bottom;
}

// Fits the design area into a window of the given client size. The same scale is used horizontally and vertically,
// so nothing is stretched: the map stays square, the buttons and the texts keep their proportions. The area is
// centered, and what is left over is filled with the background color.
void updateView(GameState& state, int clientWidth, int clientHeight) {
    const double scaleX = static_cast<double>(clientWidth) / static_cast<double>(WINDOW_WIDTH);
    const double scaleY = static_cast<double>(clientHeight) / static_cast<double>(WINDOW_HEIGHT);
    state.viewScale = std::max(0.1, std::min(scaleX, scaleY));
    state.viewOffsetX = static_cast<int>(std::lround((clientWidth - WINDOW_WIDTH * state.viewScale) / 2.0));
    state.viewOffsetY = static_cast<int>(std::lround((clientHeight - WINDOW_HEIGHT * state.viewScale) / 2.0));
}

// window pixel -> position in the design area. Everything else (buttons, map cells, hover) works in design
// coordinates, so it does not need to know the window size.
POINT toDesignPoint(const GameState& state, int pixelX, int pixelY) {
    POINT p{};
    p.x = static_cast<LONG>(std::lround((pixelX - state.viewOffsetX) / state.viewScale));
    p.y = static_cast<LONG>(std::lround((pixelY - state.viewOffsetY) / state.viewScale));
    return p;
}

void clearPlaySelection(GameState& state) {
    state.shopItem = ShopItem::NONE;
    state.selectedTowerIndex = -1;
    state.selectedCritterIndex = -1;
}

void resetGame(GameState& state) {
    state.towers.clear();
    state.currentGroup.reset();
    state.coins = STARTING_COINS;
    state.lives = STARTING_LIVES;
    state.gameOver = false;
    state.wavesCompleted = 0;
    state.selectedTowerIndex = -1;
    state.selectedCritterIndex = -1;
    state.paused = false;
    state.speedMultiplier = 1.0;
    state.accumulator = 0.0;
    state.shots.clear();
    state.waveLog.clear();
    state.draggingPaint = false;
    state.lastPaintedCell = std::make_pair(-1, -1);
    state.demo = false;
    clearPlaySelection(state);
    setStatus(state, "Game reset.", RGB(255, 255, 255));
}

void loadBlankMap(GameState& state, int rows, int columns) {
    state.map = Map(rows, columns);
    state.selectedTowerIndex = -1;
    state.draggingPaint = false;
    state.lastPaintedCell = std::make_pair(-1, -1);
    setStatus(state, "Blank map loaded.", RGB(255, 255, 255));
}

// Gives the editor map another size and keeps what was already drawn: the path cells that are still inside the new
// size stay, and so do the entry and the exit if they are still inside. The cells of a bigger map start as scenery.
void resizeMap(GameState& state, int rows, int columns) {
    rows = std::max(MIN_DIMENSION, std::min(MAX_DIMENSION, rows));
    columns = std::max(MIN_DIMENSION, std::min(MAX_DIMENSION, columns));

    Map resized(rows, columns);
    const int keepRows = std::min(rows, state.map.getRows());
    const int keepColumns = std::min(columns, state.map.getColumns());
    for (int row = 0; row < keepRows; ++row) {
        for (int column = 0; column < keepColumns; ++column) {
            if (state.map.isPathCell(row, column)) {
                resized.setCell(row, column, CellType::PATH);
            }
        }
    }

    const std::pair<int, int> entry = state.map.getEntry();
    if (resized.isInsideMap(entry.first, entry.second)) {
        resized.setEntry(entry.first, entry.second);
    }
    const std::pair<int, int> exit = state.map.getExit();
    if (resized.isInsideMap(exit.first, exit.second)) {
        resized.setExit(exit.first, exit.second);
    }

    state.map = resized;
    state.selectedTowerIndex = -1;
    state.draggingPaint = false;
    state.lastPaintedCell = std::make_pair(-1, -1);
    setStatus(state, "Map size is now " + std::to_string(rows) + " x " + std::to_string(columns) + ".", RGB(255, 255, 255));
}

// name of the file used by a map slot, for example slot 2 -> "map2.txt"
std::string mapFileName(int slot) {
    return "map" + std::to_string(slot) + ".txt";
}

void loadSampleMap(GameState& state) {
    state.map = makeSampleMap();
    state.selectedTowerIndex = -1;
    state.draggingPaint = false;
    state.lastPaintedCell = std::make_pair(-1, -1);
    setStatus(state, "Sample map loaded.", RGB(255, 255, 255));
}

std::unique_ptr<Tower> makeTower(ShopItem item, int row, int column) {
    switch (item) {
        case ShopItem::BASIC:  return std::make_unique<BasicTower>(row, column);
        case ShopItem::SPLASH: return std::make_unique<SplashTower>(row, column);
        case ShopItem::SLOW:   return std::make_unique<SlowTower>(row, column);
        case ShopItem::NONE:   break;
    }
    return nullptr;
}

double towerRange(ShopItem item) {
    switch (item) {
        case ShopItem::BASIC:  return 3.0;
        case ShopItem::SPLASH: return 2.0;
        case ShopItem::SLOW:   return 3.0;
        case ShopItem::NONE:   return 0.0;
    }
    return 0.0;
}

// one sentence about what a shot of this tower does, shown in the panel when the tower is selected in the shop.
// The numbers are read from the tower itself so they always match the game rules.
std::string towerEffectText(const Tower& decorated) {
    const Tower& tower = baseTower(decorated); // the kind of tower, under any upgrade decorators
    if (const SplashTower* splash = dynamic_cast<const SplashTower*>(&tower)) {
        return "Hits the closest critter in range and every critter within " +
               toStringFixed(splash->getSplashRadius(), 1) + " cells of it.";
    }
    if (const SlowTower* slow = dynamic_cast<const SlowTower*>(&tower)) {
        const int percent = static_cast<int>(std::lround(slow->getSlowFactor() * 100.0));
        return "Hits the closest critter in range and slows it to " + std::to_string(percent) +
               "% speed for " + toStringFixed(slow->getSlowDuration(), 1) + " s.";
    }
    return "Hits the closest critter in range with direct damage.";
}

std::string validationMessage(const Map& map) {
    if (map.getEntry().first < 0) {
        return "No entry cell: use Entry.";
    }
    if (map.getExit().first < 0) {
        return "No exit cell: use Exit.";
    }
    if (!map.hasConnectedPath()) {
        return "Entry and exit not connected.";
    }
    return "Map is valid.";
}

COLORREF validationColor(const Map& map) {
    return map.isValidMap() ? THEME_SUCCESS : THEME_DANGER;
}

std::string editToolLabel(EditTool tool) {
    switch (tool) {
        case EditTool::PATH:    return "Path";
        case EditTool::SCENERY: return "Scenery";
        case EditTool::ENTRY:   return "Entry";
        case EditTool::EXIT:    return "Exit";
    }
    return "Path";
}

void placeTower(GameState& state, ShopItem item, int row, int column) {
    if (!state.map.isInsideMap(row, column)) {
        setStatus(state, "Click inside the map.", THEME_DANGER);
        return;
    }
    if (!state.map.isSceneryCell(row, column)) {
        setStatus(state, "Path cells cannot hold towers.", THEME_DANGER);
        return;
    }
    if (hasTowerAt(state, row, column)) {
        setStatus(state, "That cell is occupied.", THEME_DANGER);
        return;
    }

    std::unique_ptr<Tower> tower = makeTower(item, row, column);
    if (tower == nullptr) {
        return;
    }
    if (!tower->purchase(state.coins)) {
        setStatus(state, "Not enough coins.", THEME_DANGER);
        return;
    }

    const std::string towerName = tower->getName();
    state.towers.push_back(std::move(tower));
    setStatus(state, "Bought " + towerName + ".", THEME_SUCCESS);
}

void paintCell(GameState& state, int row, int column) {
    switch (state.editTool) {
        case EditTool::PATH:
            if (!state.map.setCell(row, column, CellType::PATH)) {
                setStatus(state, "This cell cannot become path.", THEME_DANGER);
            }
            break;
        case EditTool::SCENERY:
            if (!state.map.setCell(row, column, CellType::SCENERY)) {
                setStatus(state, "Entry and exit cannot become scenery.", THEME_DANGER);
            }
            break;
        case EditTool::ENTRY:
            if (!state.map.setEntry(row, column)) {
                setStatus(state, "Entry cannot equal exit.", THEME_DANGER);
            } else {
                setStatus(state, "Entry set.", RGB(255, 255, 255));
            }
            break;
        case EditTool::EXIT:
            if (!state.map.setExit(row, column)) {
                setStatus(state, "Exit cannot equal entry.", THEME_DANGER);
            } else {
                setStatus(state, "Exit set.", RGB(255, 255, 255));
            }
            break;
    }
}

void paintFromMouse(GameState& state, const Layout& layout, POINT mouse) {
    int row = 0;
    int column = 0;
    if (!isInsideMapCell(layout, mouse, state.map.getRows(), state.map.getColumns(), row, column)) {
        return;
    }
    if (state.editTool == EditTool::PATH || state.editTool == EditTool::SCENERY) {
        if (state.lastPaintedCell == std::make_pair(row, column)) {
            return;
        }
        state.lastPaintedCell = std::make_pair(row, column);
    }
    paintCell(state, row, column);
}

// Strategy pattern: the shot preview asks the tower itself, so it matches the tower's targeting strategy.
Critter* predictTarget(Tower& tower, const std::vector<Critter*>& critters) {
    return tower.selectTarget(critters);
}

COLORREF towerColor(const Tower& decorated) {
    const Tower& tower = baseTower(decorated); // upgrades (decorators) keep the color of the tower kind
    if (dynamic_cast<const BasicTower*>(&tower) != nullptr) {
        return THEME_TOWER_BASIC;
    }
    if (dynamic_cast<const SplashTower*>(&tower) != nullptr) {
        return THEME_TOWER_SPLASH;
    }
    return THEME_TOWER_SLOW;
}

COLORREF shopItemColor(ShopItem item) {
    switch (item) {
        case ShopItem::BASIC:  return THEME_TOWER_BASIC;
        case ShopItem::SPLASH: return THEME_TOWER_SPLASH;
        case ShopItem::SLOW:   return THEME_TOWER_SLOW;
        case ShopItem::NONE:   break;
    }
    return THEME_TEXT;
}

char towerLetter(const Tower& decorated) {
    const Tower& tower = baseTower(decorated);
    if (dynamic_cast<const BasicTower*>(&tower) != nullptr) {
        return 'B';
    }
    if (dynamic_cast<const SplashTower*>(&tower) != nullptr) {
        return 'P';
    }
    return 'L';
}

COLORREF critterColor(CritterType type) {
    switch (type) {
        case CritterType::MELEE:  return RGB(236, 239, 244);
        case CritterType::CASTER: return RGB(255, 120, 168);
        case CritterType::CANNON: return RGB(245, 200, 70);
        case CritterType::SUPER:  return RGB(240, 92, 92);
    }
    return THEME_TEXT;
}

// dark letters on the light critters, white letters on the dark ones
COLORREF critterTextColor(CritterType type) {
    return (type == CritterType::CASTER || type == CritterType::SUPER) ? RGB(255, 255, 255) : RGB(24, 29, 42);
}

// mixes two colors: t = 0 gives a, t = 1 gives b (GDI has no transparency, so fading is done by mixing)
COLORREF blendColor(COLORREF a, COLORREF b, double t) {
    t = std::clamp(t, 0.0, 1.0);
    auto mix = [t](int x, int y) { return static_cast<BYTE>(std::lround(x + (y - x) * t)); };
    return RGB(mix(GetRValue(a), GetRValue(b)), mix(GetGValue(a), GetGValue(b)), mix(GetBValue(a), GetBValue(b)));
}

// ---------------------------------------------------------------------------------------------------------------
// Fonts. All of them are Segoe UI (Windows' interface font) in a few sizes and weights.
// ---------------------------------------------------------------------------------------------------------------
HFONT g_fontBold = nullptr;    // body text, semibold
HFONT g_fontSmall = nullptr;   // small secondary text
HFONT g_fontLabel = nullptr;   // tiny uppercase section labels
HFONT g_fontValue = nullptr;   // numbers in the stat tiles
HFONT g_fontTitle = nullptr;   // window title
HFONT g_fontHuge = nullptr;    // GAME OVER

HFONT makeFont(int height, int weight) {
    HFONT font = CreateFontA(
        -height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    if (font == nullptr) {
        font = CreateFontA(
            -height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, "Arial");
    }
    return font;
}

// creates every font; returns the body font (g_guiFont), or nullptr if fonts cannot be created
HFONT createGuiFont() {
    g_fontBold = makeFont(14, FW_SEMIBOLD);
    g_fontSmall = makeFont(13, FW_NORMAL);
    g_fontLabel = makeFont(11, FW_BOLD);
    g_fontValue = makeFont(20, FW_BOLD);
    g_fontTitle = makeFont(19, FW_BOLD);
    g_fontHuge = makeFont(40, FW_BOLD);
    return makeFont(14, FW_NORMAL);
}

void deleteGuiFonts() {
    HFONT* fonts[] = {&g_guiFont, &g_fontBold, &g_fontSmall, &g_fontLabel, &g_fontValue, &g_fontTitle, &g_fontHuge};
    for (HFONT* font : fonts) {
        if (*font != nullptr) {
            DeleteObject(*font);
            *font = nullptr;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------
// Drawing helpers. A border of NO_COLOR means "no border", a fill of NO_COLOR means "only the border".
// ---------------------------------------------------------------------------------------------------------------
void selectPenAndBrush(HDC hdc, COLORREF fill, COLORREF border, int borderWidth, int style, HPEN& pen, HBRUSH& brush,
                       HGDIOBJ& oldPen, HGDIOBJ& oldBrush) {
    pen = (border == NO_COLOR) ? nullptr : CreatePen(style, borderWidth, border);
    brush = (fill == NO_COLOR) ? nullptr : CreateSolidBrush(fill);
    oldPen = SelectObject(hdc, pen != nullptr ? static_cast<HGDIOBJ>(pen) : GetStockObject(NULL_PEN));
    oldBrush = SelectObject(hdc, brush != nullptr ? static_cast<HGDIOBJ>(brush) : GetStockObject(NULL_BRUSH));
}

void restorePenAndBrush(HDC hdc, HPEN pen, HBRUSH brush, HGDIOBJ oldPen, HGDIOBJ oldBrush) {
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    if (pen != nullptr) {
        DeleteObject(pen);
    }
    if (brush != nullptr) {
        DeleteObject(brush);
    }
}

void drawRoundRect(HDC hdc, const RECT& rect, int radius, COLORREF fill, COLORREF border, int borderWidth = 1) {
    HPEN pen; HBRUSH brush; HGDIOBJ oldPen; HGDIOBJ oldBrush;
    selectPenAndBrush(hdc, fill, border, borderWidth, PS_SOLID, pen, brush, oldPen, oldBrush);
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius * 2, radius * 2);
    restorePenAndBrush(hdc, pen, brush, oldPen, oldBrush);
}

void drawEllipse(HDC hdc, const RECT& rect, COLORREF fill, COLORREF border, int borderWidth = 1, int style = PS_SOLID) {
    HPEN pen; HBRUSH brush; HGDIOBJ oldPen; HGDIOBJ oldBrush;
    selectPenAndBrush(hdc, fill, border, borderWidth, style, pen, brush, oldPen, oldBrush);
    Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);
    restorePenAndBrush(hdc, pen, brush, oldPen, oldBrush);
}

RECT circleRect(POINT center, int radius) {
    return RECT{center.x - radius, center.y - radius, center.x + radius, center.y + radius};
}

void drawLine(HDC hdc, POINT from, POINT to, COLORREF color, int width = 1, int style = PS_SOLID) {
    HPEN pen = CreatePen(style, width, color);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    MoveToEx(hdc, from.x, from.y, nullptr);
    LineTo(hdc, to.x, to.y);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

void drawText(HDC hdc, const RECT& rect, const std::string& text, COLORREF color,
              UINT format = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, HFONT font = nullptr) {
    HGDIOBJ oldFont = (font != nullptr) ? SelectObject(hdc, font) : nullptr;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT copy = rect;
    DrawTextA(hdc, text.c_str(), -1, &copy, format | DT_NOPREFIX);
    if (oldFont != nullptr) {
        SelectObject(hdc, oldFont);
    }
}

// width of a single line of text in the given font
int measureText(HDC hdc, const std::string& text, HFONT font) {
    HGDIOBJ oldFont = (font != nullptr) ? SelectObject(hdc, font) : nullptr;
    SIZE size{};
    GetTextExtentPoint32A(hdc, text.c_str(), static_cast<int>(text.size()), &size);
    if (oldFont != nullptr) {
        SelectObject(hdc, oldFont);
    }
    return size.cx;
}

std::string toUpper(std::string text) {
    for (char& c : text) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return text;
}

// height that a wrapped paragraph needs in the given width
int measureParagraph(HDC hdc, const std::string& text, int width, HFONT font) {
    HGDIOBJ oldFont = (font != nullptr) ? SelectObject(hdc, font) : nullptr;
    RECT measure{0, 0, width, 0};
    DrawTextA(hdc, text.c_str(), -1, &measure, DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
    if (oldFont != nullptr) {
        SelectObject(hdc, oldFont);
    }
    return measure.bottom - measure.top;
}

// small uppercase heading with a thin line after it, used to split the panel into sections
void drawSectionLabel(HDC hdc, int x, int y, int width, const std::string& text) {
    RECT r{x, y, x + width, y + 16};
    drawText(hdc, r, text, THEME_TEXT_FAINT, DT_LEFT | DT_VCENTER | DT_SINGLELINE, g_fontLabel);
    const int textWidth = measureText(hdc, text, g_fontLabel) + 8;
    if (textWidth < width) {
        drawLine(hdc, POINT{x + textWidth, y + 8}, POINT{x + width, y + 8}, THEME_DIVIDER);
    }
}

// ---------------------------------------------------------------------------------------------------------------
// Buttons
// ---------------------------------------------------------------------------------------------------------------
void drawButton(HDC hdc, const Button& button, POINT mouse) {
    if (button.isHeader) {
        drawSectionLabel(hdc, button.rect.left, button.rect.top + 4, button.rect.right - button.rect.left, button.label);
        return;
    }

    const bool hover = button.enabled && pointInRect(mouse, button.rect);
    COLORREF fill = hover ? THEME_BUTTON_HOVER : THEME_BUTTON;
    COLORREF border = THEME_BUTTON_BORDER;
    COLORREF text = THEME_TEXT;

    if (!button.enabled) {
        fill = THEME_BUTTON_DISABLED;
        border = THEME_BUTTON_DISABLED;
        text = THEME_TEXT_DISABLED;
    } else if (button.selected) {
        fill = hover ? THEME_ACCENT : THEME_ACCENT_DARK;
        border = THEME_ACCENT;
        text = RGB(255, 255, 255);
    } else if (button.style == ButtonStyle::PRIMARY) {
        fill = hover ? THEME_ACCENT : THEME_ACCENT_DARK;
        border = THEME_ACCENT;
        text = RGB(255, 255, 255);
    } else if (button.style == ButtonStyle::DANGER) {
        fill = hover ? RGB(92, 42, 52) : RGB(64, 32, 42);
        border = RGB(150, 64, 76);
        text = RGB(255, 182, 182);
    }

    drawRoundRect(hdc, button.rect, CORNER_RADIUS, fill, border);

    const bool centered = button.swatch == NO_COLOR && button.badge.empty();
    RECT content = insetRect(button.rect, centered ? 4 : 10, 0);
    if (button.swatch != NO_COLOR) {
        // colored dot that matches the tower drawn on the map
        const int cy = (button.rect.top + button.rect.bottom) / 2;
        drawEllipse(hdc, RECT{content.left, cy - 6, content.left + 12, cy + 6}, button.swatch, blendColor(button.swatch, RGB(0, 0, 0), 0.4));
        content.left += 20;
    }
    if (!button.badge.empty()) {
        const COLORREF badgeColor = !button.enabled ? THEME_TEXT_DISABLED : (button.selected ? RGB(255, 255, 255) : button.badgeColor);
        drawText(hdc, content, button.badge, badgeColor, DT_RIGHT | DT_VCENTER | DT_SINGLELINE, g_fontBold);
    }
    const UINT align = centered ? DT_CENTER : DT_LEFT;
    drawText(hdc, content, button.label, text, align | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, g_fontBold);
}

bool hasSelectedTower(const GameState& state) {
    return state.selectedTowerIndex >= 0 && state.selectedTowerIndex < static_cast<int>(state.towers.size());
}

// Strategy pattern: the targeting strategies the player cycles through with the "Target" button, in order.
const char* const TARGETING_NAMES[] = {"Nearest", "Exit", "Strongest", "Weakest"};
constexpr int TARGETING_COUNT = 4;

std::unique_ptr<TargetingStrategy> makeTargetingStrategy(int index) {
    switch (index) {
        case 1: return std::make_unique<NearestToExitStrategy>();
        case 2: return std::make_unique<StrongestCritterStrategy>();
        case 3: return std::make_unique<WeakestCritterStrategy>();
        default: return nullptr; // Tower::setTargetingStrategy(nullptr) restores NearestToTowerStrategy on the tower itself
    }
}

// index in TARGETING_NAMES of the strategy the tower uses now (read back from the tower, so nothing to keep in sync)
int targetingIndex(const Tower& tower) {
    const std::string name = tower.getTargetingStrategy().getName();
    for (int i = 0; i < TARGETING_COUNT; ++i) {
        if (name == TARGETING_NAMES[i]) {
            return i;
        }
    }
    return 0;
}

// SELECTED TOWER section: Level Up, targeting strategy, special upgrades (decorators) and Sell.
// Every action looks the tower up again by index: an upgrade replaces the unique_ptr's tower with a new decorator.
template <typename AddHeader, typename AddButton>
void addSelectedTowerButtons(GameState& state, AddHeader& addHeader, AddButton& addButton) {
    const Tower& tower = *state.towers[state.selectedTowerIndex];
    addHeader("SELECTED TOWER");

    // Decorator pattern: Level Up wraps the tower in a LevelUpDecorator
    const int upgradeCost = tower.getUpgradeCost();
    Button& upgradeButton = addButton("Level up", upgradeCost > 0 && state.coins >= upgradeCost, false, [&state]() {
        if (!hasSelectedTower(state)) {
            return;
        }
        std::unique_ptr<Tower>& selected = state.towers[state.selectedTowerIndex];
        if (levelUp(selected, state.coins)) {
            setStatus(state, std::string("Now ") + selected->getName() + ".", THEME_SUCCESS);
        } else if (selected->getLevel() >= selected->getMaxLevel()) {
            setStatus(state, "Tower already at max level.", THEME_DANGER);
        } else {
            setStatus(state, "Not enough coins.", THEME_DANGER);
        }
    });
    upgradeButton.badge = upgradeCost > 0 ? std::to_string(upgradeCost) : "MAX";
    upgradeButton.badgeColor = upgradeCost > 0 && state.coins < upgradeCost ? THEME_DANGER : THEME_GOLD;

    // Strategy pattern: each click switches the tower to the next targeting strategy
    const int current = targetingIndex(tower);
    addButton(std::string("Target: ") + TARGETING_NAMES[current], true, false, [&state]() {
        if (!hasSelectedTower(state)) {
            return;
        }
        Tower& selected = *state.towers[state.selectedTowerIndex];
        const int next = (targetingIndex(selected) + 1) % TARGETING_COUNT;
        selected.setTargetingStrategy(makeTargetingStrategy(next));
        setStatus(state, std::string("Targeting: ") + TARGETING_NAMES[next] + ".", RGB(255, 255, 255));
    });

    // Decorator pattern: the special upgrades, each one bought once
    auto addSpecialButton = [&](const std::string& label, bool owned, bool canBuy, int price,
                                bool (*buy)(std::unique_ptr<Tower>&, int&)) {
        Button& button = addButton(label, canBuy, false, [&state, buy, label]() {
            if (!hasSelectedTower(state)) {
                return;
            }
            std::unique_ptr<Tower>& selected = state.towers[state.selectedTowerIndex];
            if (buy(selected, state.coins)) {
                setStatus(state, std::string("Now ") + selected->getName() + ".", THEME_SUCCESS);
            } else {
                setStatus(state, "Cannot add " + label + ".", THEME_DANGER);
            }
        });
        button.badge = owned ? "Owned" : std::to_string(price);
        button.badgeColor = owned ? THEME_SUCCESS : (state.coins >= price ? THEME_GOLD : THEME_DANGER);
    };
    addSpecialButton("+Splash", hasDecorator<SplashDecorator>(tower), canAddSplash(tower, state.coins), SPLASH_UPGRADE_COST, addSplash);
    addSpecialButton("+Burning", hasDecorator<BurningDecorator>(tower), canAddBurning(tower, state.coins), BURNING_UPGRADE_COST, addBurning);
    addSpecialButton("+Freezing", hasDecorator<FreezingDecorator>(tower), canAddFreezing(tower, state.coins), FREEZING_UPGRADE_COST, addFreezing);

    Button& sellButton = addButton("Sell", true, false, [&state]() {
        if (!hasSelectedTower(state)) {
            return;
        }
        Tower& selected = *state.towers[state.selectedTowerIndex];
        const std::string soldName = selected.getName();
        const int refund = selected.sell(state.coins);
        if (refund > 0) {
            state.towers.erase(state.towers.begin() + state.selectedTowerIndex);
            state.selectedTowerIndex = -1;
            setStatus(state, "Sold " + soldName + ".", THEME_SUCCESS);
        }
    }, ButtonStyle::DANGER);
    sellButton.badge = "+" + std::to_string(tower.getRefundValue());
    sellButton.badgeColor = THEME_GOLD;
}

// WAVES section of the play mode: start the next wave, pause, speed, back to the editor
template <typename AddHeader, typename AddButton, typename AddButtonAt>
void addWaveButtons(GameState& state, AddHeader& addHeader, AddButton& addButton, AddButtonAt& addButtonAt) {
addHeader("WAVES");
    const int nextWave = state.wavesCompleted + 1;
    addButton("Start wave " + std::to_string(nextWave), state.currentGroup == nullptr, false, [&state, nextWave]() {
        if (state.currentGroup == nullptr) {
            try {
                state.currentGroup = std::make_unique<CritterGroup>(state.waveFactory.createGroup(nextWave, state.map));
                state.accumulator = 0.0;
                setStatus(state, "Wave " + std::to_string(nextWave) + " started.", RGB(255, 255, 255));
            } catch (const std::exception& error) {
                setStatus(state, error.what(), THEME_DANGER);
            }
        }
    }, ButtonStyle::PRIMARY, BUTTON_HEIGHT + 4);
    addButtonAt(0, 2, state.paused ? "Resume" : "Pause", true, state.paused, [&state]() {
        state.paused = !state.paused;
        setStatus(state, state.paused ? "Paused." : "Running.", RGB(255, 255, 255));
    });
    addButtonAt(1, 2, "Speed " + std::to_string(static_cast<int>(state.speedMultiplier)) + "x", true, false, [&state]() {
        if (state.speedMultiplier < 1.5) {
            state.speedMultiplier = 2.0;
        } else if (state.speedMultiplier < 3.0) {
            state.speedMultiplier = 4.0;
        } else {
            state.speedMultiplier = 1.0;
        }
        setStatus(state, "Speed set to " + std::to_string(static_cast<int>(state.speedMultiplier)) + "x.", RGB(255, 255, 255));
    });
    addButton("Back to map editor", state.currentGroup == nullptr, false, [&state]() {
        state.mode = Mode::EDIT;
        resetGame(state);
        setStatus(state, "Back to the editor.", RGB(255, 255, 255));
    });
}

std::vector<Button> buildButtons(GameState& state) {
    std::vector<Button> buttons;
    buttons.reserve(32);

    int y = BUTTON_TOP;

    // a section heading (drawn as text, it cannot be clicked)
    auto addHeader = [&](const std::string& label) {
        Button header;
        header.rect = makeRect(BUTTON_LEFT, y, BUTTON_WIDTH, SECTION_LABEL_HEIGHT);
        header.label = label;
        header.enabled = false;
        header.isHeader = true;
        buttons.push_back(std::move(header));
        y += SECTION_LABEL_HEIGHT;
    };

    // column / columns put several buttons on one row (for example 0/2 and 1/2 for two half-width buttons);
    // the row ends after the last column
    auto addButtonAt = [&](int column, int columns, const std::string& label, bool enabled, bool selected,
                           std::function<void()> action, ButtonStyle style = ButtonStyle::NORMAL,
                           int height = BUTTON_HEIGHT) -> Button& {
        const int width = (BUTTON_WIDTH - (columns - 1) * BUTTON_GAP) / columns;
        Button button;
        button.rect = makeRect(BUTTON_LEFT + column * (width + BUTTON_GAP), y, width, height);
        button.label = label;
        button.action = std::move(action);
        button.enabled = enabled;
        button.selected = selected;
        button.style = style;
        buttons.push_back(std::move(button));
        if (column == columns - 1) {
            y += height + BUTTON_GAP;
        }
        return buttons.back();
    };
    auto addButton = [&](const std::string& label, bool enabled, bool selected, std::function<void()> action,
                         ButtonStyle style = ButtonStyle::NORMAL, int height = BUTTON_HEIGHT) -> Button& {
        return addButtonAt(0, 1, label, enabled, selected, std::move(action), style, height);
    };

    if (state.mode == Mode::EDIT) {
        addHeader("TOOLS");
        addButtonAt(0, 2, "Path", true, state.editTool == EditTool::PATH, [&state]() {
            state.editTool = EditTool::PATH;
            setStatus(state, "Path tool.", RGB(255, 255, 255));
        });
        addButtonAt(1, 2, "Scenery", true, state.editTool == EditTool::SCENERY, [&state]() {
            state.editTool = EditTool::SCENERY;
            setStatus(state, "Scenery tool.", RGB(255, 255, 255));
        });
        addButtonAt(0, 2, "Entry", true, state.editTool == EditTool::ENTRY, [&state]() {
            state.editTool = EditTool::ENTRY;
            setStatus(state, "Entry tool.", RGB(255, 255, 255));
        });
        addButtonAt(1, 2, "Exit", true, state.editTool == EditTool::EXIT, [&state]() {
            state.editTool = EditTool::EXIT;
            setStatus(state, "Exit tool.", RGB(255, 255, 255));
        });

        y += SECTION_GAP;
        addHeader("MAP SIZE");
        // one click changes the size by 1, 5 or 10 rows/columns, so big maps do not need dozens of clicks
        const int step = state.sizeStep;
        const std::string stepText = step > 1 ? " " + std::to_string(step) : "";
        addButtonAt(0, 2, "Rows -" + stepText, state.map.getRows() > MIN_DIMENSION, false, [&state, step]() {
            resizeMap(state, state.map.getRows() - step, state.map.getColumns());
        });
        addButtonAt(1, 2, "Rows +" + stepText, state.map.getRows() < MAX_DIMENSION, false, [&state, step]() {
            resizeMap(state, state.map.getRows() + step, state.map.getColumns());
        });
        addButtonAt(0, 2, "Cols -" + stepText, state.map.getColumns() > MIN_DIMENSION, false, [&state, step]() {
            resizeMap(state, state.map.getRows(), state.map.getColumns() - step);
        });
        addButtonAt(1, 2, "Cols +" + stepText, state.map.getColumns() < MAX_DIMENSION, false, [&state, step]() {
            resizeMap(state, state.map.getRows(), state.map.getColumns() + step);
        });
        Button& stepButton = addButton("Size step", true, false, [&state]() {
            state.sizeStep = (state.sizeStep == 1) ? 5 : (state.sizeStep == 5 ? 10 : 1);
            setStatus(state, "Rows and Cols now change by " + std::to_string(state.sizeStep) + ".", RGB(255, 255, 255));
        });
        stepButton.badge = std::to_string(step);

        y += SECTION_GAP;
        addHeader("MAPS");
        addButtonAt(0, 2, "Blank", true, false, [&state]() {
            loadBlankMap(state, state.map.getRows(), state.map.getColumns());
        });
        addButtonAt(1, 2, "Sample", true, false, [&state]() {
            loadSampleMap(state);
        });

        // map files: pick a slot (1 to 3), then save the current map to it or load the map saved in it
        Button& slotButton = addButton("Slot " + std::to_string(state.mapSlot) + " of " + std::to_string(MAP_SLOTS), true, false, [&state]() {
            state.mapSlot = state.mapSlot + 1;
            if (state.mapSlot > MAP_SLOTS) {
                state.mapSlot = 1;
            }
            setStatus(state, "Map slot " + std::to_string(state.mapSlot) + " selected.", RGB(255, 255, 255));
        });
        slotButton.badge = mapFileName(state.mapSlot);
        slotButton.badgeColor = THEME_TEXT_MUTED;
        addButtonAt(0, 2, "Save", true, false, [&state]() {
            std::string filename = mapFileName(state.mapSlot);
            if (state.map.saveToFile(filename)) {
                setStatus(state, "Map saved to " + filename + ".", THEME_SUCCESS);
            } else {
                setStatus(state, "Could not save " + filename + ".", THEME_DANGER);
            }
        });
        addButtonAt(1, 2, "Load", true, false, [&state]() {
            std::string filename = mapFileName(state.mapSlot);
            if (state.map.loadFromFile(filename)) {
                state.selectedTowerIndex = -1;
                setStatus(state, "Loaded " + filename + ".", THEME_SUCCESS);
            } else {
                setStatus(state, "No saved map in " + filename + ".", THEME_DANGER);
            }
        });

        y += SECTION_GAP + 4;
        addButton("Play  >>", state.map.isValidMap(), false, [&state]() {
            state.mode = Mode::PLAY;
            clearPlaySelection(state);
            state.paused = false;
            state.speedMultiplier = 1.0;
            state.accumulator = 0.0;
            setStatus(state, "Play mode.", RGB(255, 255, 255));
        }, ButtonStyle::PRIMARY, BUTTON_HEIGHT + 8);
    } else if (state.gameOver) {
        // after a Game Over the only choices are to play the same map again or go back to the editor
        addHeader("GAME OVER");
        addButton("Restart game", true, false, [&state]() {
            resetGame(state);
            setStatus(state, "New game on the same map.", RGB(255, 255, 255));
        }, ButtonStyle::PRIMARY, BUTTON_HEIGHT + 4);
        addButton("Back to map editor", true, false, [&state]() {
            state.mode = Mode::EDIT;
            resetGame(state);
            setStatus(state, "Back to the editor.", RGB(255, 255, 255));
        });
    } else if (hasSelectedTower(state)) {
        // the selected tower's controls take the place of the shop (click an empty cell to get the shop back)
        addSelectedTowerButtons(state, addHeader, addButton);
        y += SECTION_GAP;
        addWaveButtons(state, addHeader, addButton, addButtonAt);
    } else {
        addHeader("SHOP");
        auto addShopButton = [&](const std::string& name, ShopItem item, int cost) {
            Button& button = addButton(name, true, state.shopItem == item, [&state, item, name]() {
                state.selectedTowerIndex = -1;
                state.shopItem = (state.shopItem == item) ? ShopItem::NONE : item;
                setStatus(state, state.shopItem == item ? name + " selected." : "Selection cleared.", RGB(255, 255, 255));
            });
            button.swatch = shopItemColor(item);
            button.badge = std::to_string(cost);
            button.badgeColor = state.coins >= cost ? THEME_GOLD : THEME_DANGER;
        };
        addShopButton("Basic", ShopItem::BASIC, 100);
        addShopButton("Splash", ShopItem::SPLASH, 150);
        addShopButton("Slow", ShopItem::SLOW, 125);

        y += SECTION_GAP;
        addWaveButtons(state, addHeader, addButton, addButtonAt);
    }

    return buttons;
}

// ---------------------------------------------------------------------------------------------------------------
// Map, towers, critters, shots
// ---------------------------------------------------------------------------------------------------------------
RECT cellRect(const Layout& layout, int row, int column) {
    // computed from both edges so neighbouring cells always touch, whatever the cell size
    const int left = layout.mapLeft + static_cast<int>(std::lround(column * layout.cellSize));
    const int top = layout.mapTop + static_cast<int>(std::lround(row * layout.cellSize));
    const int right = layout.mapLeft + static_cast<int>(std::lround((column + 1) * layout.cellSize));
    const int bottom = layout.mapTop + static_cast<int>(std::lround((row + 1) * layout.cellSize));
    return RECT{left, top, right, bottom};
}

void drawMap(HDC hdc, const GameState& state, const Layout& layout) {
    // card behind the map
    RECT card = makeRect(MAP_AREA_LEFT - 12, MAP_AREA_TOP - 12, MAP_AREA_SIZE + 24, MAP_AREA_SIZE + 24);
    drawRoundRect(hdc, card, CORNER_RADIUS, THEME_SURFACE, THEME_SURFACE_BORDER);

    // one set of brushes for the whole map instead of new ones for every cell (a map can have 1600 cells).
    // Grass cells get a faint grid; path cells have no grid so the path reads as one continuous road.
    HPEN grassPen = CreatePen(PS_SOLID, 1, THEME_GRASS_LINE);
    HPEN pathPen = CreatePen(PS_SOLID, 1, THEME_PATH);
    HBRUSH grassLight = CreateSolidBrush(THEME_GRASS_LIGHT);
    HBRUSH grassDark = CreateSolidBrush(THEME_GRASS_DARK);
    HBRUSH pathBrush = CreateSolidBrush(THEME_PATH);
    HGDIOBJ oldPen = SelectObject(hdc, grassPen);
    HGDIOBJ oldBrush = SelectObject(hdc, grassDark);

    for (int row = 0; row < state.map.getRows(); ++row) {
        for (int column = 0; column < state.map.getColumns(); ++column) {
            const RECT cell = cellRect(layout, row, column);
            const bool isRoad = state.map.isPathCell(row, column) ||
                                state.map.getEntry() == std::make_pair(row, column) ||
                                state.map.getExit() == std::make_pair(row, column);
            if (isRoad) {
                SelectObject(hdc, pathPen);
                SelectObject(hdc, pathBrush);
            } else {
                SelectObject(hdc, grassPen);
                SelectObject(hdc, ((row + column) % 2 == 0) ? grassLight : grassDark);
            }
            Rectangle(hdc, cell.left, cell.top, cell.right + 1, cell.bottom + 1);
        }
    }

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(grassPen);
    DeleteObject(pathPen);
    DeleteObject(grassLight);
    DeleteObject(grassDark);
    DeleteObject(pathBrush);

    // entry and exit: a colored pad on the road with a label that fits the cell:
    // "IN" / "OUT" in big cells, one letter in medium ones, only the pad in the small cells of a big map
    auto drawEndpoint = [&](std::pair<int, int> position, bool isEntry) {
        if (!state.map.isInsideMap(position.first, position.second)) {
            return;
        }
        const RECT cell = cellRect(layout, position.first, position.second);
        const int inset = std::max(1, static_cast<int>(layout.cellSize * 0.1));
        const COLORREF color = isEntry ? THEME_ENTRY : THEME_EXIT;
        drawRoundRect(hdc, insetRect(cell, inset, inset), CORNER_RADIUS,
                      color, blendColor(color, RGB(255, 255, 255), 0.35), layout.cellSize >= 24.0 ? 2 : 1);
        if (layout.cellSize >= 40.0) {
            drawText(hdc, cell, isEntry ? "IN" : "OUT", RGB(255, 255, 255), DT_CENTER | DT_VCENTER | DT_SINGLELINE, g_fontBold);
        } else if (layout.cellSize >= 24.0) {
            drawText(hdc, cell, isEntry ? "I" : "O", RGB(255, 255, 255), DT_CENTER | DT_VCENTER | DT_SINGLELINE, g_fontBold);
        }
    };
    drawEndpoint(state.map.getEntry(), true);
    drawEndpoint(state.map.getExit(), false);

    // thin dark frame around the grid
    const RECT grid = makeRect(layout.mapLeft, layout.mapTop, layout.mapWidth + 1, layout.mapHeight + 1);
    drawRoundRect(hdc, insetRect(grid, -1, -1), CORNER_RADIUS, NO_COLOR, THEME_MAP_FRAME, 2);

    // in the editor, show which cell the mouse would paint
    if (state.mode == Mode::EDIT) {
        int row = 0;
        int column = 0;
        if (isInsideMapCell(layout, state.mousePos, state.map.getRows(), state.map.getColumns(), row, column)) {
            drawRoundRect(hdc, cellRect(layout, row, column), CORNER_RADIUS, NO_COLOR, RGB(255, 255, 255), 2);
        }
    }
}

void drawGhost(HDC hdc, const GameState& state, const Layout& layout) {
    if (state.mode != Mode::PLAY || state.shopItem == ShopItem::NONE || state.gameOver) {
        return;
    }
    int row = 0;
    int column = 0;
    if (!isInsideMapCell(layout, state.mousePos, state.map.getRows(), state.map.getColumns(), row, column)) {
        return;
    }

    const bool allowed = state.map.isSceneryCell(row, column) && !hasTowerAt(state, row, column);
    const COLORREF color = allowed ? THEME_SUCCESS : THEME_DANGER;

    drawRoundRect(hdc, insetRect(cellRect(layout, row, column), 1, 1), CORNER_RADIUS, NO_COLOR, color, 2);

    const POINT center = cellCenter(layout, static_cast<double>(row), static_cast<double>(column));
    const int radius = static_cast<int>(std::lround(towerRange(state.shopItem) * layout.cellSize));
    drawEllipse(hdc, circleRect(center, radius), NO_COLOR, color, 1, PS_DASH);
    if (allowed) {
        const int dot = std::max(3, static_cast<int>(layout.cellSize * 0.18));
        drawEllipse(hdc, circleRect(center, dot), shopItemColor(state.shopItem), RGB(255, 255, 255));
    }
}

void drawTowers(HDC hdc, const GameState& state, const Layout& layout) {
    const int towerSize = std::max(6, static_cast<int>(std::lround(layout.cellSize * 0.72)));
    for (int index = 0; index < static_cast<int>(state.towers.size()); ++index) {
        const Tower& tower = *state.towers[index];
        if (!tower.isPurchased() || tower.isSold()) {
            continue;
        }

        const COLORREF color = towerColor(tower);
        const POINT center = cellCenter(layout, static_cast<double>(tower.getRow()), static_cast<double>(tower.getColumn()));
        const RECT body{center.x - towerSize / 2, center.y - towerSize / 2, center.x + towerSize / 2, center.y + towerSize / 2};
        const bool selected = state.selectedTowerIndex == index;

        if (selected) {
            const int radius = static_cast<int>(std::lround(tower.getRange() * layout.cellSize));
            drawEllipse(hdc, circleRect(center, radius), NO_COLOR, color, 2);
        }

        // shadow, dark base plate with a colored rim, colored turret in the middle
        RECT shadow = body;
        OffsetRect(&shadow, 0, std::max(1, towerSize / 14));
        drawRoundRect(hdc, shadow, CORNER_RADIUS, THEME_TOWER_SHADOW, NO_COLOR);
        drawRoundRect(hdc, body, CORNER_RADIUS, THEME_TOWER_BASE, selected ? RGB(255, 255, 255) : color, 2);
        const int turret = static_cast<int>(towerSize * 0.32);
        drawEllipse(hdc, circleRect(center, turret), color, blendColor(color, RGB(255, 255, 255), 0.45), 2);

        // on a big map the towers are small: the letter and the level dots are drawn only if they fit
        if (towerSize >= 20) {
            const char letter[2] = {towerLetter(tower), '\0'};
            drawText(hdc, circleRect(center, turret), letter, RGB(255, 255, 255), DT_CENTER | DT_VCENTER | DT_SINGLELINE, g_fontBold);
        }

        if (towerSize >= 28) {
            const int pip = 3;
            const int spacing = 9;
            const int levels = tower.getLevel();
            const int startX = center.x - (levels - 1) * spacing / 2;
            for (int dot = 0; dot < levels; ++dot) {
                const POINT p{startX + dot * spacing, body.bottom - 1};
                drawEllipse(hdc, circleRect(p, pip), THEME_GOLD, THEME_TOWER_BASE, 1);
            }
        }
    }
}

// true if a tower with the Burning upgrade is still burning this critter
bool isBurningCritter(const GameState& state, const Critter& critter) {
    for (const auto& tower : state.towers) {
        const BurningDecorator* burning = findDecorator<BurningDecorator>(*tower);
        if (burning != nullptr && burning->isBurning(&critter)) {
            return true;
        }
    }
    return false;
}

void drawCritters(HDC hdc, const GameState& state, const Layout& layout) {
    if (state.currentGroup == nullptr) {
        return;
    }

    const std::vector<Critter>& critters = state.currentGroup->getCritters();
    for (int index = 0; index < static_cast<int>(critters.size()); index++) {
        const Critter& critter = critters[index];
        if (!critter.isActive()) {
            continue;
        }

        const std::pair<double, double> exact = critter.getExactPosition();
        if (exact.first < 0.0 || exact.second < 0.0) {
            continue;
        }

        const POINT center = cellCenter(layout, exact.first, exact.second);
        const int radius = std::max(3, static_cast<int>(std::lround(layout.cellSize * 0.26)));
        const RECT body = circleRect(center, radius);
        const bool slowed = critter.getCurrentSpeed() < critter.getSpeed();

        // soft shadow, then the body; slowed critters get an icy blue ring
        RECT shadow = body;
        OffsetRect(&shadow, 0, std::max(1, radius / 4));
        drawEllipse(hdc, shadow, THEME_TOWER_SHADOW, NO_COLOR);
        if (slowed) {
            drawEllipse(hdc, insetRect(body, -3, -3), NO_COLOR, THEME_TOWER_SLOW, 2);
        }
        // a critter set on fire by a tower with the Burning upgrade (Decorator pattern) gets an orange ring
        if (isBurningCritter(state, critter)) {
            drawEllipse(hdc, insetRect(body, slowed ? -6 : -3, slowed ? -6 : -3), NO_COLOR, THEME_BURNING, 2);
        }
        drawEllipse(hdc, body, critterColor(critter.getType()), RGB(24, 29, 42), 2);

        // gold ring around the critter the player is inspecting
        if (index == state.selectedCritterIndex) {
            drawEllipse(hdc, insetRect(body, -6, -6), NO_COLOR, THEME_GOLD, 2);
        }

        if (radius >= 8) {
            const char symbol[2] = {critter.getSymbol(), '\0'};
            drawText(hdc, body, symbol, critterTextColor(critter.getType()), DT_CENTER | DT_VCENTER | DT_SINGLELINE, g_fontBold);
        }

        // health bar: green, then yellow, then red as the critter loses hit points
        const int healthWidth = std::max(10, static_cast<int>(std::lround(layout.cellSize * 0.5)));
        const int healthHeight = 5;
        const double ratio = critter.getMaxHitPoints() > 0
            ? std::clamp(static_cast<double>(critter.getHitPoints()) / static_cast<double>(critter.getMaxHitPoints()), 0.0, 1.0)
            : 0.0;
        const COLORREF healthColor = ratio > 0.6 ? THEME_HEALTH_HIGH : (ratio > 0.3 ? THEME_GOLD : THEME_DANGER);
        RECT back{center.x - healthWidth / 2 - 1, body.top - 10, center.x + healthWidth / 2 + 1, body.top - 10 + healthHeight + 2};
        drawRoundRect(hdc, back, CORNER_RADIUS, RGB(18, 22, 32), NO_COLOR);
        RECT fill = insetRect(back, 1, 1);
        fill.right = fill.left + static_cast<int>(std::lround((fill.right - fill.left) * ratio));
        if (fill.right > fill.left) {
            drawRoundRect(hdc, fill, CORNER_RADIUS, healthColor, NO_COLOR);
        }
    }
}

void drawShots(HDC hdc, const GameState& state) {
    for (const ShotEffect& shot : state.shots) {
        // fades from the tower's color towards the grass as the effect runs out
        const double t = std::clamp(shot.timeLeft / 0.15, 0.0, 1.0);
        const COLORREF color = blendColor(THEME_GRASS_DARK, blendColor(shot.color, RGB(255, 255, 255), 0.35), t);
        drawLine(hdc, shot.from, shot.to, color, 3);
        drawEllipse(hdc, circleRect(shot.to, 4), color, NO_COLOR);
        if (shot.splash) {
            const int radius = static_cast<int>(14 + 10 * (1.0 - t)); // the ring grows as it fades
            drawEllipse(hdc, circleRect(shot.to, radius), NO_COLOR, color, 2);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------
// Header and side panel
// ---------------------------------------------------------------------------------------------------------------
void drawHeader(HDC hdc, const GameState& state) {
    // logo: a small tower icon on an accent tile
    RECT logo = makeRect(20, 12, 32, 32);
    drawRoundRect(hdc, logo, CORNER_RADIUS, THEME_ACCENT, NO_COLOR);
    {
        const int l = logo.left;
        const int t = logo.top;
        const POINT tower[] = {
            {l + 9, t + 25}, {l + 9, t + 9}, {l + 12, t + 9}, {l + 12, t + 12}, {l + 14, t + 12}, {l + 14, t + 9},
            {l + 18, t + 9}, {l + 18, t + 12}, {l + 20, t + 12}, {l + 20, t + 9}, {l + 23, t + 9}, {l + 23, t + 25},
        };
        HPEN pen; HBRUSH brush; HGDIOBJ oldPen; HGDIOBJ oldBrush;
        selectPenAndBrush(hdc, RGB(255, 255, 255), NO_COLOR, 1, PS_SOLID, pen, brush, oldPen, oldBrush);
        Polygon(hdc, tower, static_cast<int>(sizeof(tower) / sizeof(tower[0])));
        restorePenAndBrush(hdc, pen, brush, oldPen, oldBrush);
        drawRoundRect(hdc, makeRect(l + 14, t + 18, 4, 7), CORNER_RADIUS, THEME_ACCENT, NO_COLOR);
    }

    drawText(hdc, makeRect(62, 9, 260, 22), "Tower Defense", THEME_TEXT, DT_LEFT | DT_VCENTER | DT_SINGLELINE, g_fontTitle);
    drawText(hdc, makeRect(62, 30, 260, 16), "COMP 345  |  Team VRW", THEME_TEXT_MUTED, DT_LEFT | DT_VCENTER | DT_SINGLELINE, g_fontSmall);

    // status message in a pill, with a colored dot
    const RECT pill = makeRect(MESSAGE_LEFT, MESSAGE_TOP, MESSAGE_WIDTH, MESSAGE_HEIGHT);
    drawRoundRect(hdc, pill, CORNER_RADIUS, THEME_SURFACE, THEME_SURFACE_BORDER);
    const bool neutral = state.statusColor == RGB(255, 255, 255);
    const COLORREF dotColor = neutral ? THEME_ACCENT : state.statusColor;
    const int cy = (pill.top + pill.bottom) / 2;
    drawEllipse(hdc, circleRect(POINT{pill.left + 18, cy}, 4), dotColor, NO_COLOR);
    drawText(hdc, RECT{pill.left + 32, pill.top, pill.right - 14, pill.bottom}, state.statusMessage,
             neutral ? THEME_TEXT : state.statusColor, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, g_guiFont);
}

// one tile of the stats row at the top of the panel: small label above a big value
void drawStatTile(HDC hdc, const RECT& rect, const std::string& label, const std::string& value, COLORREF valueColor) {
    drawRoundRect(hdc, rect, CORNER_RADIUS, THEME_TILE, THEME_SURFACE_BORDER);
    drawText(hdc, RECT{rect.left + 10, rect.top + 6, rect.right - 6, rect.top + 20}, label, THEME_TEXT_FAINT,
             DT_LEFT | DT_VCENTER | DT_SINGLELINE, g_fontLabel);
    drawText(hdc, RECT{rect.left + 10, rect.top + 20, rect.right - 6, rect.bottom - 4}, value, valueColor,
             DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, g_fontValue);
}

void drawStatTiles(HDC hdc, const std::vector<std::pair<std::string, std::pair<std::string, COLORREF>>>& tiles) {
    const int count = static_cast<int>(tiles.size());
    const int width = (PANEL_CONTENT_WIDTH - (count - 1) * TILE_GAP) / count;
    for (int i = 0; i < count; ++i) {
        const RECT r = makeRect(PANEL_CONTENT_LEFT + i * (width + TILE_GAP), TILES_TOP, width, TILES_HEIGHT);
        drawStatTile(hdc, r, tiles[i].first, tiles[i].second.first, tiles[i].second.second);
    }
}

// the text column to the right of the buttons. Each call adds one row and moves y down.
struct InfoWriter {
    HDC hdc;
    int x;
    int y;
    int width;

    void section(const std::string& title) {
        drawSectionLabel(hdc, x, y + 4, width, title);
        y += SECTION_LABEL_HEIGHT;
    }
    void text(const std::string& value, COLORREF color = THEME_TEXT, HFONT font = nullptr) {
        drawText(hdc, RECT{x, y, x + width, y + INFO_LINE}, value, color, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, font);
        y += INFO_LINE;
    }
    // label on the left in grey, value on the right
    void keyValue(const std::string& key, const std::string& value, COLORREF valueColor = THEME_TEXT) {
        const RECT r{x, y, x + width, y + INFO_LINE};
        drawText(hdc, r, key, THEME_TEXT_MUTED, DT_LEFT | DT_VCENTER | DT_SINGLELINE, g_guiFont);
        drawText(hdc, r, value, valueColor, DT_RIGHT | DT_VCENTER | DT_SINGLELINE, g_fontBold);
        y += INFO_LINE;
    }
    // a paragraph that wraps onto several lines (the column is narrow)
    void paragraph(const std::string& value, COLORREF color = THEME_TEXT_MUTED) {
        const int height = measureParagraph(hdc, value, width, g_fontSmall);
        drawText(hdc, RECT{x, y, x + width, y + height}, value, color, DT_LEFT | DT_TOP | DT_WORDBREAK, g_fontSmall);
        y += height + 4;
    }
    void gap(int pixels = SECTION_GAP) {
        y += pixels;
    }
};

void drawNextWave(InfoWriter& info, const GameState& state) {
    // if a wave is running, the "next" wave is the one after it
    int nextWave = state.wavesCompleted + 1;
    if (state.currentGroup != nullptr) {
        nextWave = nextWave + 1;
    }

    info.section("NEXT: WAVE " + std::to_string(nextWave));

    const CritterType types[] = {CritterType::MELEE, CritterType::CASTER, CritterType::CANNON, CritterType::SUPER};
    for (int i = 0; i < 4; i++) {
        int count = CritterGroupGenerator::countOfType(types[i], nextWave);
        if (count == 0) {
            continue; // this kind of critter does not appear in that wave
        }

        // a sample critter of that wave's level, only used to read its stats
        Critter sample(types[i], nextWave);
        const int cy = info.y + INFO_LINE / 2;
        drawEllipse(info.hdc, circleRect(POINT{info.x + 6, cy}, 5), critterColor(types[i]), RGB(24, 29, 42));
        const RECT r{info.x + 18, info.y, info.x + info.width, info.y + INFO_LINE};
        drawText(info.hdc, r, std::to_string(count) + "x " + sample.getName(), THEME_TEXT, DT_LEFT | DT_VCENTER | DT_SINGLELINE, g_guiFont);
        drawText(info.hdc, r, "HP " + std::to_string(sample.getMaxHitPoints()), THEME_TEXT_MUTED, DT_RIGHT | DT_VCENTER | DT_SINGLELINE, g_guiFont);
        info.y += INFO_LINE;
    }
}

void drawWaveLog(HDC hdc, const GameState& state) {
    const int lines = std::max(1, static_cast<int>(state.waveLog.size()));
    int y = PANEL_TOP + PANEL_HEIGHT - PANEL_PADDING - SECTION_LABEL_HEIGHT - lines * LOG_LINE;
    drawSectionLabel(hdc, PANEL_CONTENT_LEFT, y + 4, PANEL_CONTENT_WIDTH, "WAVE LOG");
    y += SECTION_LABEL_HEIGHT;

    if (state.waveLog.empty()) {
        drawText(hdc, makeRect(PANEL_CONTENT_LEFT, y, PANEL_CONTENT_WIDTH, LOG_LINE), "No finished waves yet.", THEME_TEXT_FAINT,
                 DT_LEFT | DT_VCENTER | DT_SINGLELINE, g_fontSmall);
        return;
    }

    for (const std::string& lineText : state.waveLog) {
        drawText(hdc, makeRect(PANEL_CONTENT_LEFT, y, PANEL_CONTENT_WIDTH, LOG_LINE), lineText, THEME_TEXT_MUTED,
                 DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, g_fontSmall);
        y += LOG_LINE;
    }
}

void drawPanel(HDC hdc, GameState& state) {
    const RECT panel = makeRect(PANEL_LEFT, PANEL_TOP, PANEL_WIDTH, PANEL_HEIGHT);
    drawRoundRect(hdc, panel, CORNER_RADIUS, THEME_SURFACE, THEME_SURFACE_BORDER);

    // panel heading: a colored mode pill on the left, a short state on the right
    const bool edit = state.mode == Mode::EDIT;
    const COLORREF modeColor = state.gameOver ? THEME_DANGER : THEME_ACCENT;
    const RECT modePill = makeRect(PANEL_CONTENT_LEFT, PANEL_TOP + 16, 96, 24);
    drawRoundRect(hdc, modePill, CORNER_RADIUS, blendColor(THEME_SURFACE, modeColor, 0.22), modeColor);
    drawText(hdc, modePill, edit ? "EDIT MODE" : "PLAY MODE", blendColor(modeColor, RGB(255, 255, 255), 0.45),
             DT_CENTER | DT_VCENTER | DT_SINGLELINE, g_fontLabel);

    const RECT headingRight = makeRect(PANEL_CONTENT_LEFT + 110, PANEL_TOP + 16, PANEL_CONTENT_WIDTH - 110, 24);
    if (edit) {
        drawText(hdc, headingRight, "Design a map, then press Play", THEME_TEXT_MUTED, DT_RIGHT | DT_VCENTER | DT_SINGLELINE, g_fontSmall);
    } else {
        std::string runState = state.gameOver ? "Game over" : (state.paused ? "Paused" : "Running");
        if (!state.gameOver && !state.paused && state.speedMultiplier > 1.5) {
            runState += "  " + std::to_string(static_cast<int>(state.speedMultiplier)) + "x";
        }
        const COLORREF runColor = state.gameOver ? THEME_DANGER : (state.paused ? THEME_GOLD : THEME_SUCCESS);
        drawText(hdc, headingRight, runState, runColor, DT_RIGHT | DT_VCENTER | DT_SINGLELINE, g_fontBold);
        const int textWidth = measureText(hdc, runState, g_fontBold);
        drawEllipse(hdc, circleRect(POINT{headingRight.right - textWidth - 12, (headingRight.top + headingRight.bottom) / 2}, 4), runColor, NO_COLOR);
    }

    if (edit) {
        drawStatTiles(hdc, {
            {"ROWS", {std::to_string(state.map.getRows()), THEME_TEXT}},
            {"COLUMNS", {std::to_string(state.map.getColumns()), THEME_TEXT}},
            {"TOOL", {editToolLabel(state.editTool), THEME_TEXT}},
            {"MAP", {state.map.isValidMap() ? "Valid" : "Invalid", validationColor(state.map)}},
        });
    } else {
        drawStatTiles(hdc, {
            {"COINS", {std::to_string(state.coins), THEME_GOLD}},
            {"LIVES", {std::to_string(state.lives) + "/" + std::to_string(STARTING_LIVES), state.lives <= 3 ? THEME_DANGER : THEME_TEXT}},
            {"WAVES", {std::to_string(state.wavesCompleted), THEME_TEXT}},
            {"TOWERS", {std::to_string(state.towers.size()), THEME_TEXT}},
        });
    }

    InfoWriter info{hdc, INFO_LEFT, BUTTON_TOP, INFO_WIDTH};

    if (edit) {
        info.section("MAP CHECK");
        const COLORREF checkColor = validationColor(state.map);
        drawEllipse(hdc, circleRect(POINT{info.x + 5, info.y + INFO_LINE / 2}, 4), checkColor, NO_COLOR);
        drawText(hdc, RECT{info.x + 16, info.y, info.x + info.width, info.y + INFO_LINE}, validationMessage(state.map), checkColor,
                 DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, g_fontBold);
        info.y += INFO_LINE;
        info.paragraph("Click or drag on the map to paint with the selected tool.");

        info.gap();
        info.section("LEGEND");
        auto legend = [&](COLORREF color, const std::string& name, const std::string& note) {
            const RECT swatch = makeRect(info.x, info.y + 3, 14, 14);
            drawRoundRect(hdc, swatch, CORNER_RADIUS, color, blendColor(color, RGB(0, 0, 0), 0.3));
            const RECT r{info.x + 22, info.y, info.x + info.width, info.y + INFO_LINE};
            drawText(hdc, r, name, THEME_TEXT, DT_LEFT | DT_VCENTER | DT_SINGLELINE, g_guiFont);
            drawText(hdc, r, note, THEME_TEXT_FAINT, DT_RIGHT | DT_VCENTER | DT_SINGLELINE, g_fontSmall);
            info.y += INFO_LINE;
        };
        legend(THEME_PATH, "Path", "critters walk");
        legend(THEME_GRASS_LIGHT, "Scenery", "towers go here");
        legend(THEME_ENTRY, "Entry", "critters spawn");
        legend(THEME_EXIT, "Exit", "costs a life");

        info.gap();
        info.section("MAP FILES");
        info.paragraph("Pick a slot, then Save or Load. Maps are kept in map1.txt to map" + std::to_string(MAP_SLOTS) + ".txt.");
    } else {
        // current wave with a progress bar of how many critters have entered the map
        if (state.currentGroup != nullptr) {
            const CritterGroup& group = *state.currentGroup;
            info.section("WAVE " + std::to_string(group.getWaveNumber()) + " IN PROGRESS");
            const RECT bar = makeRect(info.x, info.y + 4, info.width, 8);
            drawRoundRect(hdc, bar, CORNER_RADIUS, THEME_TILE, NO_COLOR);
            const double done = group.getSize() > 0 ? static_cast<double>(group.getKilledCount() + group.getEscapedCount()) / group.getSize() : 0.0;
            RECT barFill = bar;
            barFill.right = bar.left + static_cast<int>(std::lround(info.width * std::clamp(done, 0.0, 1.0)));
            if (barFill.right - barFill.left >= 8) {
                drawRoundRect(hdc, barFill, CORNER_RADIUS, THEME_ACCENT, NO_COLOR);
            }
            info.gap(16);
            info.keyValue("Spawned", std::to_string(group.getSpawnedCount()) + " / " + std::to_string(group.getSize()));
            info.keyValue("Killed / escaped", std::to_string(group.getKilledCount()) + " / " + std::to_string(group.getEscapedCount()));
            info.keyValue("Earned / stolen", "+" + std::to_string(group.getCoinsEarned()) + " / -" + std::to_string(group.getCoinsStolen()), THEME_GOLD);
        } else {
            info.section("WAVE");
            info.text(state.gameOver ? "The game is over." : "Waiting. Press Start wave.", THEME_TEXT_MUTED, g_guiFont);
        }

        info.gap();
        std::unique_ptr<Tower> preview = makeTower(state.shopItem, 0, 0); // only used to read the stats
        if (preview != nullptr) {
            // a tower type is selected in the shop: describe it before the player buys it
            info.section("SHOP: " + toUpper(preview->getName()));
            info.keyValue("Cost", std::to_string(preview->getCost()), state.coins >= preview->getCost() ? THEME_GOLD : THEME_DANGER);
            info.keyValue("Damage", std::to_string(preview->getDamage()));
            info.keyValue("Range", toStringFixed(preview->getRange(), 1));
            info.keyValue("Fire rate", toStringFixed(preview->getFireRate(), 1) + "/s");
            info.paragraph(towerEffectText(*preview));
            if (state.coins < preview->getCost()) {
                info.text("Not enough coins.", THEME_DANGER, g_fontBold);
            } else {
                info.paragraph("Click a scenery cell to place it.", THEME_SUCCESS);
            }
        } else if (state.selectedTowerIndex >= 0 && state.selectedTowerIndex < static_cast<int>(state.towers.size())) {
            const Tower& tower = *state.towers[state.selectedTowerIndex];
            info.section(toUpper(tower.getName()));
            info.keyValue("Level", std::to_string(tower.getLevel()) + " / " + std::to_string(tower.getMaxLevel()), THEME_GOLD);
            info.keyValue("Damage", std::to_string(tower.getDamage()));
            info.keyValue("Range", toStringFixed(tower.getRange(), 1));
            info.keyValue("Fire rate", toStringFixed(tower.getFireRate(), 1) + "/s");
            info.keyValue("Targeting", tower.getTargetingStrategy().getName());
            info.keyValue("Refund", std::to_string(tower.getRefundValue()), THEME_GOLD);
            if (const BurningDecorator* burning = findDecorator<BurningDecorator>(tower)) {
                info.keyValue("Burning", std::to_string(burning->getBurningCount()) + " critters", THEME_DANGER);
            }
            if (const FreezingDecorator* freezing = findDecorator<FreezingDecorator>(tower)) {
                info.keyValue("Freezing", std::to_string(static_cast<int>(std::lround(freezing->getSlowFactor() * 100.0))) + "% speed",
                              THEME_TOWER_SLOW);
            }
            if (const SplashDecorator* splash = findDecorator<SplashDecorator>(tower)) {
                info.keyValue("Splash", std::to_string(splash->getSplashDamage()) + " in " + toStringFixed(splash->getSplashRadius(), 1),
                              THEME_TOWER_SPLASH);
            }
        } else if (state.currentGroup != nullptr && state.selectedCritterIndex >= 0) {
            // stats of the critter the player clicked on
            const Critter& critter = state.currentGroup->getCritters()[state.selectedCritterIndex];
            info.section(toUpper(critter.getName()) + " - LEVEL " + std::to_string(critter.getLevel()));
            if (critter.isActive()) {
                info.keyValue("HP", std::to_string(critter.getHitPoints()) + " / " + std::to_string(critter.getMaxHitPoints()));
            } else if (critter.hasEscaped()) {
                info.text("Reached the exit", THEME_DANGER, g_fontBold);
            } else {
                info.text("Destroyed", THEME_SUCCESS, g_fontBold);
            }
            const bool slowed = critter.getCurrentSpeed() < critter.getSpeed();
            info.keyValue("Speed", toStringFixed(critter.getCurrentSpeed(), 2) + (slowed ? " (slowed)" : ""), slowed ? THEME_TOWER_SLOW : THEME_TEXT);
            info.keyValue("Strength", std::to_string(critter.getStrength()));
            info.keyValue("Reward", std::to_string(critter.getReward()), THEME_GOLD);
        } else if (!state.gameOver) {
            info.section("SELECTION");
            info.paragraph("Click a tower or a critter to see its stats, or pick a tower in the shop.");
        }

        if (!state.gameOver) {
            info.gap();
            drawNextWave(info, state);
        }

        drawWaveLog(hdc, state);
    }

    for (const Button& button : buildButtons(state)) {
        drawButton(hdc, button, state.mousePos);
    }
}

// draws a dark card with "GAME OVER" in the middle of the map
void drawGameOver(HDC hdc, const GameState& state, const Layout& layout) {
    if (!state.gameOver) {
        return;
    }

    const int centerX = layout.mapLeft + layout.mapWidth / 2;
    const int centerY = layout.mapTop + layout.mapHeight / 2;
    const RECT box = makeRect(centerX - 210, centerY - 80, 420, 160);
    RECT shadow = box;
    OffsetRect(&shadow, 0, 6);
    drawRoundRect(hdc, shadow, CORNER_RADIUS, RGB(8, 10, 16), NO_COLOR);
    drawRoundRect(hdc, box, CORNER_RADIUS, THEME_SURFACE, THEME_DANGER, 2);

    drawText(hdc, makeRect(box.left, box.top + 18, 420, 52), "GAME OVER", THEME_DANGER, DT_CENTER | DT_VCENTER | DT_SINGLELINE, g_fontHuge);

    const std::string summary = "You survived " + std::to_string(state.wavesCompleted) + " wave(s).";
    drawText(hdc, makeRect(box.left, box.top + 78, 420, 24), summary, THEME_TEXT, DT_CENTER | DT_VCENTER | DT_SINGLELINE, g_fontBold);
    drawText(hdc, makeRect(box.left, box.top + 108, 420, 22), "Press Restart game or Back to map editor.", THEME_TEXT_MUTED,
             DT_CENTER | DT_VCENTER | DT_SINGLELINE, g_fontSmall);
}

void drawScene(HDC hdc, GameState& state, const Layout& layout) {
    HGDIOBJ oldFont = nullptr;
    if (g_guiFont != nullptr) {
        oldFont = SelectObject(hdc, g_guiFont);
    }

    drawHeader(hdc, state);
    drawMap(hdc, state, layout);

    // range circles can be bigger than the map: everything on the board is clipped to the map grid
    const int savedDC = SaveDC(hdc);
    IntersectClipRect(hdc, layout.mapLeft, layout.mapTop, layout.mapLeft + layout.mapWidth + 1, layout.mapTop + layout.mapHeight + 1);
    drawGhost(hdc, state, layout);
    drawShots(hdc, state);
    drawTowers(hdc, state, layout);
    drawCritters(hdc, state, layout);
    RestoreDC(hdc, savedDC);

    drawGameOver(hdc, state, layout);
    drawPanel(hdc, state);

    if (oldFont != nullptr) {
        SelectObject(hdc, oldFont);
    }
}

bool saveScreenshot(HWND hwnd, const wchar_t* filename) {
    RECT client{};
    GetClientRect(hwnd, &client);
    const int width = client.right - client.left;
    const int height = client.bottom - client.top;

    HDC screenDC = GetDC(hwnd);
    if (screenDC == nullptr) {
        return false;
    }

    HDC memoryDC = CreateCompatibleDC(screenDC);
    if (memoryDC == nullptr) {
        ReleaseDC(hwnd, screenDC);
        return false;
    }

    HBITMAP bitmap = CreateCompatibleBitmap(screenDC, width, height);
    if (bitmap == nullptr) {
        DeleteDC(memoryDC);
        ReleaseDC(hwnd, screenDC);
        return false;
    }

    HGDIOBJ oldBitmap = SelectObject(memoryDC, bitmap);
    BitBlt(memoryDC, 0, 0, width, height, screenDC, 0, 0, SRCCOPY | CAPTUREBLT);
    SelectObject(memoryDC, oldBitmap);
    DeleteDC(memoryDC);
    ReleaseDC(hwnd, screenDC);

    IWICImagingFactory* factory = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        DeleteObject(bitmap);
        return false;
    }

    IWICBitmap* wicBitmap = nullptr;
    hr = factory->CreateBitmapFromHBITMAP(bitmap, nullptr, WICBitmapUseAlpha, &wicBitmap);
    DeleteObject(bitmap);
    if (FAILED(hr)) {
        factory->Release();
        return false;
    }

    IWICStream* stream = nullptr;
    hr = factory->CreateStream(&stream);
    if (SUCCEEDED(hr)) {
        hr = stream->InitializeFromFilename(filename, GENERIC_WRITE);
    }

    IWICBitmapEncoder* encoder = nullptr;
    if (SUCCEEDED(hr)) {
        hr = factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
    }
    if (SUCCEEDED(hr)) {
        hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
    }

    IWICBitmapFrameEncode* frame = nullptr;
    IPropertyBag2* properties = nullptr;
    if (SUCCEEDED(hr)) {
        hr = encoder->CreateNewFrame(&frame, &properties);
    }
    if (SUCCEEDED(hr)) {
        hr = frame->Initialize(properties);
    }
    if (SUCCEEDED(hr)) {
        hr = frame->SetSize(static_cast<UINT>(width), static_cast<UINT>(height));
    }
    if (SUCCEEDED(hr)) {
        WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
        hr = frame->SetPixelFormat(&format);
    }
    if (SUCCEEDED(hr)) {
        hr = frame->WriteSource(wicBitmap, nullptr);
    }
    if (SUCCEEDED(hr)) {
        hr = frame->Commit();
    }
    if (SUCCEEDED(hr)) {
        hr = encoder->Commit();
    }

    if (properties != nullptr) {
        properties->Release();
    }
    if (frame != nullptr) {
        frame->Release();
    }
    if (encoder != nullptr) {
        encoder->Release();
    }
    if (stream != nullptr) {
        stream->Release();
    }
    if (wicBitmap != nullptr) {
        wicBitmap->Release();
    }
    factory->Release();

    return SUCCEEDED(hr);
}

void logFinishedWave(GameState& state) {
    if (state.currentGroup == nullptr || !state.currentGroup->isFinished()) {
        return;
    }

    const int waveNumber = state.currentGroup->getWaveNumber();
    const int killed = state.currentGroup->getKilledCount();
    const int escaped = state.currentGroup->getEscapedCount();
    const int earned = state.currentGroup->getCoinsEarned();
    const int stolen = state.currentGroup->getCoinsStolen();

    std::ostringstream out;
    out << "Wave " << waveNumber << ": " << killed << " killed (+" << earned << "), " << escaped << " escaped (-" << stolen << ")";
    state.waveLog.push_back(out.str());
    if (state.waveLog.size() > 5) {
        state.waveLog.erase(state.waveLog.begin());
    }

    state.wavesCompleted++;
    state.currentGroup.reset();
    state.selectedCritterIndex = -1; // the selected critter belonged to the wave that just ended
    state.accumulator = 0.0;
    setStatus(state, out.str(), RGB(255, 255, 255));
}

void autoManageDemo(GameState& state) {
    if (!state.demo || state.mode != Mode::PLAY || state.gameOver) {
        return;
    }
    if (state.currentGroup != nullptr) {
        return;
    }
    if (state.wavesCompleted >= DEMO_WAVES) {
        setStatus(state, "Demo complete.", RGB(255, 255, 255));
        return;
    }

    const struct {
        int row;
        int column;
        ShopItem item;
    } plan[] = {
        {3, 3, ShopItem::BASIC},
        {3, 4, ShopItem::SPLASH},
        {1, 3, ShopItem::SLOW},
        {3, 2, ShopItem::BASIC},
    };

    for (const auto& step : plan) {
        if (hasTowerAt(state, step.row, step.column)) {
            continue;
        }
        std::unique_ptr<Tower> tower = makeTower(step.item, step.row, step.column);
        if (tower != nullptr && tower->purchase(state.coins)) {
            state.towers.push_back(std::move(tower));
        }
    }

    bool upgraded = true;
    while (upgraded) {
        upgraded = false;
        for (auto& tower : state.towers) {
            if (levelUp(tower, state.coins)) { // Decorator pattern: one more LevelUpDecorator
                upgraded = true;
            }
        }
    }

    try {
        state.currentGroup = std::make_unique<CritterGroup>(state.waveFactory.createGroup(state.wavesCompleted + 1, state.map));
        state.accumulator = 0.0;
    } catch (const std::exception& error) {
        setStatus(state, error.what(), THEME_DANGER);
    }
}

void updateSimulation(GameState& state, double frameDelta) {
    const double clamped = std::min(frameDelta, MAX_FRAME_DELTA);

    if (state.mode == Mode::PLAY && state.currentGroup != nullptr && !state.paused && !state.gameOver) {
        state.accumulator += clamped * state.speedMultiplier;
        while (state.accumulator >= FIXED_STEP && state.currentGroup != nullptr) {
            std::vector<Critter*> activeCritters = state.currentGroup->getActiveCritters();

            for (auto& tower : state.towers) {
                if (!tower->isPurchased() || tower->isSold()) {
                    continue;
                }

                Critter* target = predictTarget(*tower, activeCritters);
                const bool fired = tower->update(FIXED_STEP, activeCritters);
                if (fired && target != nullptr) {
                    ShotEffect effect;
                    effect.from = cellCenter(computeLayout(state.map), static_cast<double>(tower->getRow()), static_cast<double>(tower->getColumn()));
                    const std::pair<double, double> exact = target->getExactPosition();
                    effect.to = cellCenter(computeLayout(state.map), exact.first, exact.second);
                    effect.splash = dynamic_cast<const SplashTower*>(&baseTower(*tower)) != nullptr
                                    || hasDecorator<SplashDecorator>(*tower);
                    effect.color = towerColor(*tower);
                    state.shots.push_back(effect);
                }
            }

            // every critter that reaches the exit during this update costs the player one life
            int escapedBefore = state.currentGroup->getEscapedCount();
            state.currentGroup->update(FIXED_STEP, state.map, state.coins);
            int newEscapes = state.currentGroup->getEscapedCount() - escapedBefore;
            state.lives = state.lives - newEscapes;
            state.accumulator -= FIXED_STEP;

            if (state.lives <= 0) {
                state.lives = 0;
                state.gameOver = true;
                setStatus(state, "GAME OVER: too many critters reached the exit.", THEME_DANGER);
                break; // stop the simulation, the board stays frozen behind the Game Over screen
            }

            logFinishedWave(state);
        }
    }

    for (ShotEffect& shot : state.shots) {
        shot.timeLeft -= clamped;
    }
    state.shots.erase(std::remove_if(state.shots.begin(), state.shots.end(), [](const ShotEffect& shot) {
        return shot.timeLeft <= 0.0;
    }), state.shots.end());

    autoManageDemo(state);
}

// returns the index of the critter under the mouse, or -1 if there is none.
// Critters move smoothly between cells, so we compare the mouse with the drawn position of each critter.
int findCritterAt(const GameState& state, const Layout& layout, POINT mouse) {
    if (state.currentGroup == nullptr) {
        return -1;
    }

    const std::vector<Critter>& critters = state.currentGroup->getCritters();
    int bestIndex = -1;
    double bestDistance = std::max(layout.cellSize * 0.5, 10.0); // the click must be within half a cell of the critter (at least 10 px on big maps)

    for (int i = 0; i < static_cast<int>(critters.size()); i++) {
        if (!critters[i].isActive()) {
            continue;
        }

        std::pair<double, double> exact = critters[i].getExactPosition();
        POINT center = cellCenter(layout, exact.first, exact.second);
        double dx = mouse.x - center.x;
        double dy = mouse.y - center.y;
        double distance = std::sqrt(dx * dx + dy * dy);

        if (distance < bestDistance) {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    return bestIndex;
}

void handleMapClick(GameState& state, POINT mouse) {
    const Layout layout = computeLayout(state.map);
    int row = 0;
    int column = 0;
    if (!isInsideMapCell(layout, mouse, state.map.getRows(), state.map.getColumns(), row, column)) {
        if (state.mode == Mode::PLAY) {
            clearPlaySelection(state);
        }
        return;
    }

    if (state.mode == Mode::EDIT) {
        if (state.editTool == EditTool::PATH || state.editTool == EditTool::SCENERY) {
            state.draggingPaint = true;
            state.lastPaintedCell = std::make_pair(-1, -1);
            paintFromMouse(state, layout, mouse);
        } else {
            paintCell(state, row, column);
        }
        return;
    }

    // the game is over, clicks on the map do nothing until the player restarts
    if (state.gameOver) {
        return;
    }

    if (state.shopItem != ShopItem::NONE) {
        placeTower(state, state.shopItem, row, column);
        return;
    }

    for (int index = 0; index < static_cast<int>(state.towers.size()); ++index) {
        const auto& tower = state.towers[index];
        if (tower->isPurchased() && !tower->isSold() && tower->getRow() == row && tower->getColumn() == column) {
            state.selectedTowerIndex = index;
            state.selectedCritterIndex = -1;
            setStatus(state, std::string(tower->getName()) + " (click an empty cell for the shop)", RGB(255, 255, 255));
            return;
        }
    }

    int critterIndex = findCritterAt(state, layout, mouse);
    if (critterIndex >= 0) {
        state.selectedCritterIndex = critterIndex;
        state.selectedTowerIndex = -1;
        setStatus(state, "Inspecting a critter.", RGB(255, 255, 255));
        return;
    }

    clearPlaySelection(state);
}

bool handleButtonClick(GameState& state, POINT mouse) {
    std::vector<Button> buttons = buildButtons(state);
    for (const Button& button : buttons) {
        if (button.enabled && pointInRect(mouse, button.rect)) {
            button.action();
            return true;
        }
    }
    return false;
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    GameState* state = reinterpret_cast<GameState*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

    switch (message) {
        case WM_CREATE: {
            CREATESTRUCTA* create = reinterpret_cast<CREATESTRUCTA*>(lParam);
            state = reinterpret_cast<GameState*>(create->lpCreateParams);
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
            state->hwnd = hwnd;
            SetTimer(hwnd, 1, 16, nullptr);
            return 0;
        }
        case WM_TIMER:
            if (state != nullptr) {
                updateSimulation(*state, 1.0 / 60.0);
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        case WM_MOUSEMOVE:
            if (state != nullptr) {
                state->mousePos = toDesignPoint(*state, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                if (state->mode == Mode::EDIT && state->draggingPaint) {
                    paintFromMouse(*state, computeLayout(state->map), state->mousePos);
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            }
            return 0;
        case WM_LBUTTONDOWN:
            if (state != nullptr) {
                state->mousePos = toDesignPoint(*state, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                if (!handleButtonClick(*state, state->mousePos)) {
                    handleMapClick(*state, state->mousePos);
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            SetCapture(hwnd);
            return 0;
        case WM_LBUTTONUP:
            if (state != nullptr) {
                state->draggingPaint = false;
                state->lastPaintedCell = std::make_pair(-1, -1);
            }
            ReleaseCapture();
            return 0;
        case WM_RBUTTONDOWN:
            if (state != nullptr) {
                clearPlaySelection(*state);
                state->draggingPaint = false;
                state->lastPaintedCell = std::make_pair(-1, -1);
                setStatus(*state, "Selection cleared.", RGB(255, 255, 255));
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        case WM_KEYDOWN:
            if (state != nullptr) {
                switch (wParam) {
                    case VK_ESCAPE:
                        clearPlaySelection(*state);
                        state->draggingPaint = false;
                        setStatus(*state, "Selection cleared.", RGB(255, 255, 255));
                        InvalidateRect(hwnd, nullptr, FALSE);
                        return 0;
                    case VK_SPACE:
                        if (state->mode == Mode::PLAY) {
                            state->paused = !state->paused;
                            setStatus(*state, state->paused ? "Paused." : "Running.", RGB(255, 255, 255));
                            InvalidateRect(hwnd, nullptr, FALSE);
                        }
                        return 0;
                    case VK_F12:
                        if (saveScreenshot(hwnd, L"screenshot.png")) {
                            setStatus(*state, "Saved screenshot.png.", RGB(255, 255, 255));
                        } else {
                            setStatus(*state, "Could not save screenshot.png.", THEME_DANGER);
                        }
                        InvalidateRect(hwnd, nullptr, FALSE);
                        return 0;
                    default:
                        break;
                }
            }
            break;
        case WM_SIZE:
            if (state != nullptr && wParam != SIZE_MINIMIZED) {
                updateView(*state, LOWORD(lParam), HIWORD(lParam));
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        case WM_GETMINMAXINFO: {
            // below this size the texts would be too small to read
            MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMinTrackSize.x = MIN_WINDOW_WIDTH;
            info->ptMinTrackSize.y = MIN_WINDOW_HEIGHT;
            return 0;
        }
        case WM_ERASEBKGND:
            return 1; // WM_PAINT draws the whole window, erasing first would only make it flicker
        case WM_PAINT: {
            PAINTSTRUCT ps{};
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT client{};
            GetClientRect(hwnd, &client);

            HDC memoryDC = CreateCompatibleDC(hdc);
            HBITMAP backBuffer = CreateCompatibleBitmap(hdc, client.right - client.left, client.bottom - client.top);
            HGDIOBJ oldBitmap = SelectObject(memoryDC, backBuffer);

            HBRUSH bg = CreateSolidBrush(THEME_BACKGROUND);
            FillRect(memoryDC, &client, bg);
            DeleteObject(bg);

            if (state != nullptr) {
                updateView(*state, client.right - client.left, client.bottom - client.top);

                // draw in design coordinates: the transform scales and centers everything (shapes, texts, lines)
                SetGraphicsMode(memoryDC, GM_ADVANCED);
                XFORM transform{};
                transform.eM11 = static_cast<FLOAT>(state->viewScale);
                transform.eM22 = static_cast<FLOAT>(state->viewScale);
                transform.eDx = static_cast<FLOAT>(state->viewOffsetX);
                transform.eDy = static_cast<FLOAT>(state->viewOffsetY);
                SetWorldTransform(memoryDC, &transform);

                const Layout layout = computeLayout(state->map);
                drawScene(memoryDC, *state, layout);

                ModifyWorldTransform(memoryDC, nullptr, MWT_IDENTITY); // plain pixels again for the copy to the window
            }

            BitBlt(hdc, 0, 0, client.right - client.left, client.bottom - client.top, memoryDC, 0, 0, SRCCOPY);
            SelectObject(memoryDC, oldBitmap);
            DeleteObject(backBuffer);
            DeleteDC(memoryDC);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }

    return DefWindowProcA(hwnd, message, wParam, lParam);
}

int runApplication(bool demo, int nCmdShow) {
    GameState state;
    state.demo = demo;
    if (demo) {
        loadSampleMap(state);
        state.mode = Mode::PLAY;
        state.speedMultiplier = 4.0;
        setStatus(state, "Demo mode.", RGB(255, 255, 255));
    }

    HRESULT comInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(comInit)) {
        MessageBoxA(nullptr, "Could not initialize COM.", "COMP 345 Tower Defense", MB_OK | MB_ICONERROR);
        return 1;
    }

    g_guiFont = createGuiFont();
    if (g_guiFont == nullptr) {
        MessageBoxA(nullptr, "Could not create the GUI font.", "COMP 345 Tower Defense", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return 1;
    }

    HINSTANCE instance = GetModuleHandleA(nullptr);
    const char* className = "COMP345_TowerDefense_Win32";

    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = windowProc;
    wc.hInstance = instance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (RegisterClassExA(&wc) == 0) {
        MessageBoxA(nullptr, "Could not register the window class.", "COMP 345 Tower Defense", MB_OK | MB_ICONERROR);
        deleteGuiFonts();
        CoUninitialize();
        return 1;
    }

    // outer size so that the inside of the window is exactly the design area, limited to the free screen area
    RECT frame{0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&frame, WS_OVERLAPPEDWINDOW, FALSE);
    int windowWidth = frame.right - frame.left;
    int windowHeight = frame.bottom - frame.top;
    RECT workArea{};
    if (SystemParametersInfoA(SPI_GETWORKAREA, 0, &workArea, 0)) {
        windowWidth = std::min(windowWidth, static_cast<int>(workArea.right - workArea.left));
        windowHeight = std::min(windowHeight, static_cast<int>(workArea.bottom - workArea.top));
    }

    HWND hwnd = CreateWindowExA(
        0,
        className,
        "COMP 345 Tower Defense",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowWidth,
        windowHeight,
        nullptr,
        nullptr,
        instance,
        &state);

    if (hwnd == nullptr) {
        MessageBoxA(nullptr, "Could not create the game window.", "COMP 345 Tower Defense", MB_OK | MB_ICONERROR);
        deleteGuiFonts();
        CoUninitialize();
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    if (g_guiFont != nullptr) {
        deleteGuiFonts();
    }

    CoUninitialize();

    return static_cast<int>(msg.wParam);
}

bool commandLineHasDemo() {
    const char* cmd = GetCommandLineA();
    return cmd != nullptr && std::strstr(cmd, "--demo") != nullptr;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
    return runApplication(commandLineHasDemo(), SW_SHOW);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int nCmdShow) {
    return runApplication(commandLineHasDemo(), nCmdShow);
}

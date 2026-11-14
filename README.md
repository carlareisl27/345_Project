# COMP 345 Project (Team VRW): Tower Defense (Assignment 3: Strategy, Factory, Decorator)

C++17 tower defense game. Assignment 3 adds three design patterns to the existing map / tower / critter code:

| Part | Pattern | Main files | Driver | Tests |
|------|---------|------------|--------|-------|
| 1 | Strategy, tower targeting | `Towers/TargetingStrategy.h`, `Towers/NearestToTowerStrategy.*`, `Towers/NearestToExitStrategy.*`, `Towers/StrongestCritterStrategy.*`, `Towers/WeakestCritterStrategy.*`, `Towers/Tower.*` | `Towers/TowerStrategyDriver.cpp` | `Towers/TowerStrategyTests.cpp` |
| 2 | Factory, critter waves | `Critters/CritterFactory.h`, `Critters/StandardCritterFactory.*` | `Critters/CritterFactoryDriver.cpp` | `Critters/CritterFactoryTests.cpp` |
| 3 | Decorator, tower upgrades | `Towers/TowerDecorator.*`, `Towers/TowerUpgrades.*`, `Towers/LevelUpDecorator.*`, `Towers/SplashDecorator.*`, `Towers/BurningDecorator.*`, `Towers/FreezingDecorator.*` | `Towers/TowerDecoratorDriver.cpp` | `Towers/TowerDecoratorTests.cpp` |

`GameDriver.cpp` plays 10 waves using all three patterns together. The analysis (game rules) and design are in
`TowerDefense/Report.dox` (Doxygen main page: run `doxygen` in `TowerDefense/`, then open `html/index.html`).

## Requirements

- g++ with C++17 support (tested with g++ 11; MinGW-w64 g++ on Windows)
- GNU make (optional; the plain g++ commands are below)

## Build and run with make

From the `TowerDefense/` folder:

```
make            # builds every driver and test program (zero warnings with -std=c++17 -Wall -Wextra)
make patterns   # runs the three Assignment 3 drivers
make test       # runs all 7 test programs
./game_driver   # the integrated game (game_driver.exe on Windows)
make gui        # Windows only: the Win32 graphical driver
make clean
```

## Build without make

From the `TowerDefense/` folder:

```
FLAGS="-std=c++17 -Wall -Wextra -I . -I Map -I Towers -I Critters"
CORE="Map/Map.cpp Critters/Critter.cpp Critters/CritterGroup.cpp Critters/CritterGroupGenerator.cpp Critters/StandardCritterFactory.cpp"
TOWERS="Towers/Tower.cpp Towers/NearestToTowerStrategy.cpp Towers/NearestToExitStrategy.cpp Towers/StrongestCritterStrategy.cpp Towers/WeakestCritterStrategy.cpp Towers/TowerDecorator.cpp Towers/LevelUpDecorator.cpp Towers/SplashDecorator.cpp Towers/BurningDecorator.cpp Towers/FreezingDecorator.cpp Towers/TowerUpgrades.cpp"

g++ $FLAGS $CORE $TOWERS Towers/TowerStrategyDriver.cpp   -o tower_strategy_driver
g++ $FLAGS $CORE         Critters/CritterFactoryDriver.cpp -o critter_factory_driver
g++ $FLAGS $CORE $TOWERS Towers/TowerDecoratorDriver.cpp  -o tower_decorator_driver
g++ $FLAGS $CORE $TOWERS Towers/TowerStrategyTests.cpp    -o tower_strategy_tests
g++ $FLAGS $CORE         Critters/CritterFactoryTests.cpp  -o critter_factory_tests
g++ $FLAGS $CORE $TOWERS Towers/TowerDecoratorTests.cpp   -o tower_decorator_tests
g++ $FLAGS $CORE $TOWERS GameDriver.cpp                   -o game_driver
```

The earlier drivers and tests (map, tower, critter, observers) build the same way; their targets are in the `Makefile`.

## What each Assignment 3 driver shows

- **tower_strategy_driver**: one Basic Tower and the same four critters; the tower's strategy is swapped with
  `setTargetingStrategy` and each of the four strategies picks a different critter. Then the tower shoots with the
  weakest-critter strategy.
- **critter_factory_driver**: waves 1, 5 and 10 from `StandardCritterFactory` (count, level, spawn interval, count per type)
  and the difficulty curve up to wave 28.
- **tower_decorator_driver**: stats before/after each decorator, stacked decorators, levels bought with `levelUp()` (next level cost, refund, damage and range at each level, refusals at the maximum level), then a fully upgraded tower
  (`Basic Tower Lv2 +Splash +Freezing +Burning`) shooting critters step by step.

## GUI controls (`make gui`, then `gui_driver.exe`)

Edit the map, press **Play >>**, buy towers from the shop, then click a tower to replace the shop with its controls
(click an empty cell to get the shop back):

- **Level up** (price, or MAX): Decorator pattern; wraps the tower in a `LevelUpDecorator` through `levelUp()` (cost x level).
- **Target: Nearest / Exit / Strongest / Weakest**: Strategy pattern; each click switches to the next targeting strategy.
- **+Splash** (150), **+Burning** (125), **+Freezing** (100): Decorator pattern; each one can be bought once per tower.
- **Sell**: refunds half of what was paid for the tower and its levels.

The stats panel shows the decorated name (for example `Basic Tower Lv2 +Burning`), the level, damage, range, fire rate,
targeting and refund. Burning critters get an orange ring and slowed critters a blue one.

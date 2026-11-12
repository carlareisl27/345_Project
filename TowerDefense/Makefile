# make           builds every driver and test
# make run       runs the four drivers
# make observers runs the three observer drivers (Tower, Map, Critter observers)
# make test      runs the four test programs
# make gui    builds the Windows GUI driver
# make clean  removes the programs
# (without make, the g++ commands are in ../README.md)
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I . -I Map -I Towers -I Critters

ifeq ($(OS),Windows_NT)
EXE = .exe
RM = del /F /Q
RM_REDIRECT = 2>NUL
else
EXE =
RM = rm -f
RM_REDIRECT =
endif

MAP_SRC     = Map/Map.cpp
CRITTER_SRC = Critters/Critter.cpp Critters/CritterGroup.cpp Critters/CritterGroupGenerator.cpp
TOWER_SRC   = Towers/Tower.cpp
ALL_SRC     = $(MAP_SRC) $(CRITTER_SRC) $(TOWER_SRC)
OBSERVER_SRC = Towers/TowerObserver.cpp Map/MapObserver.cpp Critters/CritterObserver.cpp

PROGRAMS = map_driver tower_driver critter_driver game_driver map_tests tower_tests critter_tests \
           tower_observer_driver map_observer_driver critter_observer_driver observer_tests
GUI_PROGRAMS = gui_driver

all: $(PROGRAMS)

map_driver: $(MAP_SRC) Map/Main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

tower_driver: $(ALL_SRC) Towers/TowerDriver.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

critter_driver: $(MAP_SRC) $(CRITTER_SRC) Critters/CritterDriver.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

game_driver: $(ALL_SRC) GameDriver.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

map_tests: $(MAP_SRC) Map/MapTests.cpp TestHelper.h
	$(CXX) $(CXXFLAGS) $(MAP_SRC) Map/MapTests.cpp -o $@

tower_tests: $(ALL_SRC) Towers/TowerTests.cpp TestHelper.h
	$(CXX) $(CXXFLAGS) $(ALL_SRC) Towers/TowerTests.cpp -o $@

critter_tests: $(MAP_SRC) $(CRITTER_SRC) Critters/CritterTests.cpp TestHelper.h
	$(CXX) $(CXXFLAGS) $(MAP_SRC) $(CRITTER_SRC) Critters/CritterTests.cpp -o $@

tower_observer_driver: $(ALL_SRC) Towers/TowerObserver.cpp Towers/TowerObserverDriver.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

map_observer_driver: $(MAP_SRC) Map/MapObserver.cpp Map/MapObserverDriver.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

critter_observer_driver: $(MAP_SRC) $(CRITTER_SRC) Critters/CritterObserver.cpp Critters/CritterObserverDriver.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

observer_tests: $(ALL_SRC) $(OBSERVER_SRC) ObserverTests.cpp TestHelper.h Observer.h
	$(CXX) $(CXXFLAGS) $(ALL_SRC) $(OBSERVER_SRC) ObserverTests.cpp -o $@

gui_driver: $(ALL_SRC) Gui/WinGui.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -mwindows -lgdi32 -luser32 -lwindowscodecs -lole32 -luuid

gui: gui_driver

run: all
	./map_driver
	./tower_driver
	./critter_driver
	./game_driver

observers: tower_observer_driver map_observer_driver critter_observer_driver
	./tower_observer_driver
	./map_observer_driver
	./critter_observer_driver

test: map_tests tower_tests critter_tests observer_tests
	./map_tests
	./tower_tests
	./critter_tests
	./observer_tests

clean:
	-$(RM) $(addsuffix $(EXE),$(PROGRAMS) $(GUI_PROGRAMS)) $(RM_REDIRECT)

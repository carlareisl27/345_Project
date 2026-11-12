#include "Tower.h"
#include "Critter.h"
#include "Map.h"

#include <iostream>

using namespace std;

int main(){
    int coins = 1000;

    BasicTower basicTower(1, 1);
    SplashTower splashTower(2, 3);
    SlowTower slowTower(4, 2);

    // test purchasing a tower
    if(basicTower.purchase(coins)){
        cout << "You now own a Basic Tower!\n";
    } else {
        cout << "Purchase unsuccessful!\n";
    }

    cout << "Coins remaining: " << coins << "\n";
    cout << "Tower level: " << basicTower.getLevel() << "\n";
    cout << "Tower damage: " <<basicTower.getDamage() << "\n";
    cout << "Tower range: " << basicTower.getRange() << "\n";

    // test upgrading a tower
    if(basicTower.upgrade(coins)){
        cout << "You have upgraded your Basic Tower!\n";
    } else {
        cout << "Upgrade failed!\n";
    }

    cout << "Coins remaining: " << coins << "\n";
    cout << "Tower level: " << basicTower.getLevel() << "\n";
    cout << "Tower damage: " <<basicTower.getDamage() << "\n";
    cout << "Tower range: " << basicTower.getRange() << "\n";

    // test purchasing a splash tower
    if(splashTower.purchase(coins)){
        cout << "You now own a Splash Tower!\n";
    } else {
        cout << "Purchase unsuccessful!\n";
    }

    cout << "Coins remaining: " << coins << "\n";

    // test purchasing a slow tower
    if(slowTower.purchase(coins)){
        cout << "You now own a Slow Tower!\n";
    } else {
        cout << "Purchase unsuccessful!\n";
    }

    cout << "Coins remaining: " << coins << "\n";

    // test selling basic tower
    int refund = basicTower.sell(coins);

    if(refund > 0){
        cout << "You have sold your Basic Tower!\n";
        cout << "Basic Tower price: " << refund << " coins\n";
    } else {
        cout << "Sale unsuccessful!\n";
    }

    cout << "Coins remaining: " << coins << "\n";

    // test to upgrade a sold tower
    if (basicTower.upgrade(coins)){
        cout << "You have upgraded the Tower!\n";
    } else {
        cout << "Update unsuccessful!\n";
    }

    // test attacking critters: the towers detect critters in range, select one and shoot it
    Map map(3, 10);
    map.setEntry(0, 0);
    map.setExit(0, 9);
    for(int column = 1; column < 9; column++){
        map.setCell(0, column, CellType::PATH);
    }

    Critter near(CritterType::CANNON, 1);
    Critter next(CritterType::CANNON, 1);
    Critter far(CritterType::CANNON, 1);
    near.spawn(map);
    next.spawn(map);
    far.spawn(map);
    near.update(4.0 / near.getSpeed());
    next.update(5.0 / next.getSpeed());
    far.update(8.0 / far.getSpeed());
    vector<Critter*> critters = {&near, &next, &far};

    BasicTower attacker(1, 4);
    SplashTower splasher(1, 4);
    SlowTower slower(1, 4);
    int attackCoins = 1000;
    attacker.purchase(attackCoins);
    splasher.purchase(attackCoins);
    slower.purchase(attackCoins);

    cout << "\nCritters at columns 4, 5 and 8 (60 HP each), towers at column 4 (range 3 for Basic and Slow)\n";
    attacker.update(0.0, critters);
    cout << "Basic Tower shot the closest critter in range: HP " << near.getHitPoints() << ", " << next.getHitPoints() << ", " << far.getHitPoints() << "\n";
    cout << "Shooting again right away: " << (attacker.update(0.0, critters) ? "shot" : "cooldown, no shot") << "\n";

    cout << "Critter speed before slowing: " << near.getCurrentSpeed() << "\n";
    slower.update(0.0, critters);
    cout << "Slow Tower damaged and slowed its target: HP " << near.getHitPoints() << ", speed " << near.getCurrentSpeed() << "\n";

    splasher.update(0.0, critters);
    cout << "Splash Tower hurt the critters close to its target: HP " << near.getHitPoints() << ", " << next.getHitPoints() << ", " << far.getHitPoints() << "\n";

    return 0;

}

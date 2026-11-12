#include "TowerObserver.h"
#include "Tower.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace {
    const int VIEW_WIDTH = 44; // characters between the two borders

    std::string border() {
        return "+" + std::string(VIEW_WIDTH, '-') + "+";
    }

    // one line of the view: "| label : value     |"
    std::string line(const std::string& label, const std::string& value) {
        std::ostringstream text;
        text << " " << std::left << std::setw(13) << label << ": " << value;
        std::string content = text.str();
        if (static_cast<int>(content.size()) > VIEW_WIDTH) {
            content = content.substr(0, VIEW_WIDTH);
        }
        return "|" + content + std::string(VIEW_WIDTH - content.size(), ' ') + "|";
    }

    std::string title(const std::string& text) {
        std::string content = " " + text;
        return "|" + content + std::string(VIEW_WIDTH - content.size(), ' ') + "|";
    }

    std::string number(double value, int decimals) {
        std::ostringstream text;
        text << std::fixed << std::setprecision(decimals) << value;
        return text.str();
    }
}

TowerObserver::TowerObserver(Tower* tower, std::ostream& out)
: tower(tower), out(out), refreshCount(0) {
    if (this->tower != nullptr) {
        this->tower->attach(this); // plugs the view to the tower
    }
}

TowerObserver::~TowerObserver() {
    if (tower != nullptr) {
        tower->detach(this);
    }
}

void TowerObserver::update(Observable* subject) {
    // only the tower this view was made for is displayed
    if (subject == tower) {
        display();
    }
}

void TowerObserver::subjectDestroyed(Observable* subject) {
    if (subject == tower) {
        tower = nullptr;
    }
}

void TowerObserver::display() {
    refreshCount++;

    if (tower == nullptr) {
        out << "[Tower view] the tower no longer exists.\n";
        return;
    }

    std::string status = tower->isSold() ? "sold" : (tower->isPurchased() ? "purchased" : "in the shop (not bought)");
    std::string upgrade = tower->getUpgradeCost() > 0 ? std::to_string(tower->getUpgradeCost()) + " coins"
                        : (tower->getLevel() >= tower->getMaxLevel() ? "max level" : "not available");

    out << border() << '\n';
    out << title("TOWER VIEW  (refresh #" + std::to_string(refreshCount) + ")") << '\n';
    out << border() << '\n';
    out << line("Type", tower->getName()) << '\n';
    out << line("Cell", "(" + std::to_string(tower->getRow()) + ", " + std::to_string(tower->getColumn()) + ")") << '\n';
    out << line("Status", status) << '\n';
    out << line("Level", std::to_string(tower->getLevel()) + " / " + std::to_string(tower->getMaxLevel())) << '\n';
    out << line("Damage", std::to_string(tower->getDamage())) << '\n';
    out << line("Range", number(tower->getRange(), 1) + " cells") << '\n';
    out << line("Fire rate", number(tower->getFireRate(), 2) + " shots/s") << '\n';
    out << line("Cooldown", number(tower->getCooldown(), 2) + " s") << '\n';
    out << line("Buy cost", std::to_string(tower->getCost()) + " coins") << '\n';
    out << line("Upgrade cost", upgrade) << '\n';
    out << line("Refund", std::to_string(tower->getRefundValue()) + " coins") << '\n';

    // the special values of each type of tower
    if (const SplashTower* splash = dynamic_cast<const SplashTower*>(tower)) {
        out << line("Splash radius", number(splash->getSplashRadius(), 1) + " cells") << '\n';
    }
    if (const SlowTower* slow = dynamic_cast<const SlowTower*>(tower)) {
        out << line("Slow", "x" + number(slow->getSlowFactor(), 2) + " speed for " + number(slow->getSlowDuration(), 1) + " s") << '\n';
    }

    out << border() << "\n\n";
}

int TowerObserver::getRefreshCount() const {
    return refreshCount;
}

const Tower* TowerObserver::getTower() const {
    return tower;
}

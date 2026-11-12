#include "CritterObserver.h"
#include "Critter.h"
#include "CritterGroup.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace {
    const char* stateName(CritterState state) {
        switch (state) {
            case CritterState::WAITING: return "WAITING";
            case CritterState::ACTIVE: return "ACTIVE";
            case CritterState::DEAD: return "DEAD";
            case CritterState::ESCAPED: return "ESCAPED";
        }
        return "?";
    }

    // hit point bar: [######----]
    std::string healthBar(int hitPoints, int maxHitPoints, int length = 20) {
        int filled = maxHitPoints > 0 ? (hitPoints * length + maxHitPoints - 1) / maxHitPoints : 0;
        if (filled > length) {
            filled = length;
        }
        return "[" + std::string(filled, '#') + std::string(length - filled, '-') + "]";
    }

    std::string number(double value, int decimals) {
        std::ostringstream text;
        text << std::fixed << std::setprecision(decimals) << value;
        return text.str();
    }

    std::string cell(const Critter& critter) {
        if (critter.getRow() < 0) {
            return "-";
        }
        return "(" + std::to_string(critter.getRow()) + "," + std::to_string(critter.getColumn()) + ")";
    }
}

// ---------------------------------------------------------------- CritterObserver

CritterObserver::CritterObserver(Critter* critter, std::ostream& out)
: critter(critter), out(out), refreshCount(0) {
    if (this->critter != nullptr) {
        this->critter->attach(this); // plugs the view to the critter
    }
}

CritterObserver::~CritterObserver() {
    if (critter != nullptr) {
        critter->detach(this);
    }
}

void CritterObserver::update(Observable* subject) {
    if (subject == critter) {
        display();
    }
}

void CritterObserver::subjectDestroyed(Observable* subject) {
    if (subject == critter) {
        critter = nullptr;
    }
}

void CritterObserver::display() {
    refreshCount++;

    if (critter == nullptr) {
        out << "[Critter view] the critter no longer exists.\n";
        return;
    }

    std::string speed = number(critter->getSpeed(), 2) + " cells/s";
    if (critter->getSlowTimeLeft() > 0.0) {
        speed += " (slowed to " + number(critter->getCurrentSpeed(), 2) + " for " + number(critter->getSlowTimeLeft(), 1) + " s)";
    }

    std::string progress = "(not on the map)";
    if (critter->getPathIndex() >= 0) {
        progress = "step " + std::to_string(critter->getPathIndex()) + " of " + std::to_string(critter->getPathLength() - 1);
    }

    out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    out << " CRITTER VIEW  (refresh #" << refreshCount << ")\n";
    out << " " << critter->getName() << " critter [" << critter->getSymbol() << "], level " << critter->getLevel() << '\n';
    out << "   State     : " << stateName(critter->getState()) << '\n';
    out << "   Cell      : " << (critter->getRow() < 0 ? std::string() : cell(*critter) + "   ") << progress << '\n';
    out << "   Hit points: " << std::setw(3) << critter->getHitPoints() << " / " << std::setw(3) << critter->getMaxHitPoints()
        << "  " << healthBar(critter->getHitPoints(), critter->getMaxHitPoints()) << '\n';
    out << "   Speed     : " << speed << '\n';
    out << "   Strength  : " << critter->getStrength() << " (coins stolen at the exit)\n";
    out << "   Reward    : " << critter->getReward() << " coins\n";
    out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
}

int CritterObserver::getRefreshCount() const {
    return refreshCount;
}

const Critter* CritterObserver::getCritter() const {
    return critter;
}

// ---------------------------------------------------------------- CritterGroupObserver

CritterGroupObserver::CritterGroupObserver(CritterGroup* group, std::ostream& out)
: group(group), out(out), refreshCount(0) {
    if (this->group != nullptr) {
        this->group->attach(this); // plugs the view to the group
    }
}

CritterGroupObserver::~CritterGroupObserver() {
    if (group != nullptr) {
        group->detach(this);
    }
}

void CritterGroupObserver::update(Observable* subject) {
    if (subject == group) {
        display();
    }
}

void CritterGroupObserver::subjectDestroyed(Observable* subject) {
    if (subject == group) {
        group = nullptr;
    }
}

void CritterGroupObserver::display() {
    refreshCount++;

    if (group == nullptr) {
        out << "[Critter group view] the group no longer exists.\n";
        return;
    }

    const std::vector<Critter>& critters = group->getCritters();

    out << "==============================================================\n";
    out << " CRITTER GROUP VIEW  (refresh #" << refreshCount << ")  wave " << group->getWaveNumber()
        << ", " << critters.size() << " critter" << (critters.size() == 1 ? "" : "s") << '\n';
    out << "--------------------------------------------------------------\n";
    out << "   #  Type    Sym  Lvl    HP         State     Cell\n";

    for (std::size_t i = 0; i < critters.size(); i++) {
        const Critter& critter = critters[i];
        std::string hitPoints = std::to_string(critter.getHitPoints()) + "/" + std::to_string(critter.getMaxHitPoints());
        out << "  " << std::setw(2) << i << "  " << std::left << std::setw(8) << critter.getName()
            << critter.getSymbol() << "    " << std::setw(5) << critter.getLevel()
            << std::setw(11) << hitPoints << std::setw(10) << stateName(critter.getState())
            << cell(critter) << std::right << '\n';
    }

    if (critters.empty()) {
        out << "  (no critters in this wave)\n";
    }

    out << "--------------------------------------------------------------\n";
    out << " Total HP " << group->getTotalHitPoints() << "   spawned " << group->getSpawnedCount()
        << "   killed " << group->getKilledCount() << "   escaped " << group->getEscapedCount()
        << "   coins +" << group->getCoinsEarned() << " / -" << group->getCoinsStolen() << '\n';
    out << "==============================================================\n\n";
}

int CritterGroupObserver::getRefreshCount() const {
    return refreshCount;
}

const CritterGroup* CritterGroupObserver::getGroup() const {
    return group;
}

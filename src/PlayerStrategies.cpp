#include "PlayerStrategies.h"

#include "CardOrderIntegration.h"
#include "Cards.h"
#include "Map.h"
#include "Orders.h"
#include "Player.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>


// ==================================================
// HumanPlayerStrategy
// ==================================================
HumanPlayerStrategy::HumanPlayerStrategy(Player* player) : PlayerStrategy(player) {}
HumanPlayerStrategy::HumanPlayerStrategy(const HumanPlayerStrategy& other) : PlayerStrategy(other) {}
HumanPlayerStrategy& HumanPlayerStrategy::operator=(const HumanPlayerStrategy& other) {
    if (this != &other) {
        PlayerStrategy::operator=(other);
    }
    return *this;
}
HumanPlayerStrategy::~HumanPlayerStrategy() {}
void HumanPlayerStrategy::issueOrder()  {
    // Implementation for human player order issuance
}
vector<Territory*> HumanPlayerStrategy::toAttack()  {
    // Implementation for human player attack selection
    return {};
}
vector<Territory*> HumanPlayerStrategy::toDefend()  {
    // Implementation for human player defense selection
    return {};
}
PlayerStrategy* HumanPlayerStrategy::clone() const  {
    return new HumanPlayerStrategy(*this);
}
ostream& operator<<(ostream& out, const HumanPlayerStrategy& hps) {
    out << "Human Player Strategy";
    return out;
}




// ==================================================
// CheaterPlayerStrategy
// ==================================================
CheaterPlayerStrategy::CheaterPlayerStrategy(Player* player) : PlayerStrategy(player) {}
CheaterPlayerStrategy::CheaterPlayerStrategy(const CheaterPlayerStrategy& other) : PlayerStrategy(other) {}
CheaterPlayerStrategy& CheaterPlayerStrategy::operator=(const CheaterPlayerStrategy& other) {
    if (this != &other) {
        PlayerStrategy::operator=(other);
    }
    return *this;
}
CheaterPlayerStrategy::~CheaterPlayerStrategy() {}
void CheaterPlayerStrategy::issueOrder()  {
    // Implementation for cheater player order issuance
}
vector<Territory*> CheaterPlayerStrategy::toAttack()  {
    // Implementation for cheater player attack selection
    return {};
}
vector<Territory*> CheaterPlayerStrategy::toDefend()  {
    // Implementation for cheater player defense selection
    return {};
}
PlayerStrategy* CheaterPlayerStrategy::clone() const  {
    return new CheaterPlayerStrategy(*this);
}
ostream& operator<<(ostream& out, const CheaterPlayerStrategy& cps) {
    out << "Cheater Player Strategy";
    return out;
}
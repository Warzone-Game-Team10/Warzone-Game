#include "Issue28Driver.h"
#include "Orders.h"
#include "Player.h"
#include "Cards.h"
#include <iostream>
#include <stdexcept>

namespace {
void require(bool value, const char* description) {
    if (!value) throw std::runtime_error(description);
    std::cout << "PASS: " << description << '\n';
}
}

bool testIssue28() {
    std::cout << "\n=== ISSUE #28: COMPLETE ORDER EXECUTION DRIVER ===\n";
    // 1. Validation and real execution for Bomb, including invalid cases.
    Player alice, bob;
    alice.setName("Alice"); bob.setName("Bob");
    Territory a(810, "Alice border"), b(811, "Bob border"), far(812, "Far away");
    a.setArmyCount(20); b.setArmyCount(9); far.setArmyCount(8);
    a.addAdjacentTerritory(&b);
    alice.addTerritory(&a); bob.addTerritory(&b); bob.addTerritory(&far);
    Territory* ownedA = alice.getTerritoryList()[0];
    Territory* ownedB = bob.getTerritoryList()[0];
    Bomb bomb(&alice, ownedB);
    require(bomb.validate(), "Bomb accepts an adjacent enemy");
    require(!Bomb(&alice, ownedA).validate() &&
            !Bomb(&alice, bob.getTerritoryList()[1]).validate(),
            "Bomb rejects friendly and non-adjacent targets");
    bomb.execute();
    require(ownedB->getArmyCount() == 4, "Bomb halves nine defending armies (integer division)");

    // 2. Negotiation prevents Advance attacks in both directions.
    Negotiate diplomacy(&alice, &bob);
    require(diplomacy.validate() && !Negotiate(&alice, &alice).validate(),
            "Negotiate accepts an enemy player but rejects self");
    diplomacy.execute();
    Advance blocked(&alice, ownedA, ownedB, 3);
    require(!blocked.validate(), "Negotiation prevents an Advance attack");
    alice.resetTurnState();
    require(blocked.validate(), "Negotiation expires at the end of the turn");

    // 3. Blockade moves the actual territory to Neutral, doubling armies.
    Territory c(813, "Blockade target"); c.setArmyCount(7);
    alice.addTerritory(&c);
    Territory* ownedC = alice.getTerritoryList().back();
    Blockade blockade(&alice, ownedC);
    require(blockade.validate() && !Blockade(&bob, ownedC).validate(),
            "Blockade accepts only issuer-owned territory");
    blockade.execute();
    require(!alice.ownsTerritory(ownedC) && ownedC->getOwner() != &alice &&
            ownedC->getOwner() && ownedC->getOwner()->getName() == "Neutral" &&
            ownedC->getArmyCount() == 14,
            "Blockade doubles armies and transfers ownership to Neutral");

    // 4. Deploy, friendly Advance, and Airlift all execute and change armies.
    Territory d(814, "Friendly destination");
    alice.addTerritory(&d);
    Territory* ownedD = alice.getTerritoryList().back();
    ownedA->addAdjacentTerritory(ownedD);
    alice.setReinforcementPool(5);
    Deploy deploy(&alice, ownedA, 5);
    require(deploy.validate(), "Deploy validates owned target and reinforcement pool");
    deploy.execute();
    require(ownedA->getArmyCount() == 25 && alice.getReinforcementPool() == 0,
            "Deploy adds five armies and spends reinforcements");
    Advance friendly(&alice, ownedA, ownedD, 5);
    require(friendly.validate(), "Advance validates friendly adjacency");
    friendly.execute();
    require(ownedA->getArmyCount() == 20 && ownedD->getArmyCount() == 5,
            "Advance moves armies to an adjacent friendly territory");
    Airlift airlift(&alice, ownedD, ownedA, 3);
    require(airlift.validate(), "Airlift validates two owned territories without adjacency requirement");
    airlift.execute();
    require(ownedA->getArmyCount() == 23 && ownedD->getArmyCount() == 2,
            "Airlift transfers armies between owned territories");

    // 5. Deterministic conquest: undefended territory is captured immediately.
    Territory undefended(815, "Undefended enemy");
    undefended.setArmyCount(0);
    ownedA->addAdjacentTerritory(&undefended);
    bob.addTerritory(&undefended);
    Territory* enemy = bob.getTerritoryList().back();
    Advance conquest(&alice, ownedA, enemy, 4);
    require(conquest.validate(), "Conquest order validates before execution");
    conquest.execute();
    require(alice.ownsTerritory(enemy) && !bob.ownsTerritory(enemy) &&
            enemy->getOwner() == &alice && alice.hasConqueredThisTurn(),
            "Conquest transfers territory ownership and records a conquest");

    // 6. Award one card per turn, even after multiple conquests.
    Deck deck;
    const int initialDeck = deck.getSize();
    const int initialHand = alice.getCardHand()->getSize();
    require(alice.awardConquestCard(deck), "First conquest earns a card at end of turn");
    require(!alice.awardConquestCard(deck) &&
            deck.getSize() == initialDeck - 1 &&
            alice.getCardHand()->getSize() == initialHand + 1,
            "Multiple conquests cannot award a second card in the same turn");
    alice.resetTurnState();
    require(!alice.awardConquestCard(deck), "New turn without conquest earns no card");
    // 7. Issue every concrete order through the player-owned OrdersList and
    // execute them polymorphically, as a game engine would.
    Player queuedPlayer, queuedOpponent;
    queuedPlayer.setName("Queue issuer"); queuedOpponent.setName("Queue opponent");
    Territory qa(820, "Queue source"), qb(821, "Queue friendly"),
              qc(822, "Queue enemy"), qd(823, "Queue blockade");
    qa.setArmyCount(15); qb.setArmyCount(0); qc.setArmyCount(8); qd.setArmyCount(2);
    qa.addAdjacentTerritory(&qb); qa.addAdjacentTerritory(&qc);
    queuedPlayer.addTerritory(&qa); queuedPlayer.addTerritory(&qb);
    queuedPlayer.addTerritory(&qd); queuedOpponent.addTerritory(&qc);
    Territory* qs = queuedPlayer.getTerritoryList()[0];
    Territory* qf = queuedPlayer.getTerritoryList()[1];
    Territory* qblock = queuedPlayer.getTerritoryList()[2];
    Territory* qe = queuedOpponent.getTerritoryList()[0];
    queuedPlayer.setReinforcementPool(2);
    OrdersList* queue = queuedPlayer.getOrdersList();
    queue->addOrder(new Deploy(&queuedPlayer, qs, 2));
    queue->addOrder(new Advance(&queuedPlayer, qs, qf, 2));
    queue->addOrder(new Airlift(&queuedPlayer, qf, qs, 1));
    queue->addOrder(new Bomb(&queuedPlayer, qe));
    queue->addOrder(new Blockade(&queuedPlayer, qblock));
    queue->addOrder(new Negotiate(&queuedPlayer, &queuedOpponent));
    for (int i = 0; i < 6; ++i) {
        Order* order = queue->getOrder(i);
        require(order && order->validate(), "Queued order validated before execution");
        order->execute();
    }
    require(qe->getArmyCount() == 4 && qblock->getOwner()->getName() == "Neutral" &&
            queuedPlayer.isNegotiatingWith(queuedOpponent),
            "All six player-issued orders execute through a polymorphic queue");
    std::cout << "ALL ISSUE #28 DRIVER CHECKS PASSED\n";
    return true;
}

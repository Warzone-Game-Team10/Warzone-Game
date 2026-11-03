#include "CardOrderIntegration.h"
#include <iostream>

int main() {
    Deck deck;

    Player alice("Alice", nullptr, nullptr, {});
    Player bob("Bob", nullptr, nullptr, {});

    Hand* hand = alice.getCardHand();
    OrdersList* orders = alice.getOrdersList();

    // Alice's default hand has no deck.
    // Give her a hand connected to our deck.
    Hand connectedHand(&deck);
    alice.setCardHand(&connectedHand);

    hand = alice.getCardHand();

    Card* card = new Card(CardType::DIPLOMACY);
    hand->addCard(card);

    const int initialHandSize = hand->getSize();
    const int initialDeckSize = deck.getSize();

    // Test 1: Wrong issuer
    bool wrongIssuer = playCardWithOrder(
        card, &alice, new Negotiate(&bob, &alice)
    );

    if (wrongIssuer ||
        hand->getSize() != initialHandSize ||
        deck.getSize() != initialDeckSize) {
        std::cerr << "FAIL: Wrong issuer test.\n";
        return 1;
    }

    std::cout << "PASS: Wrong issuer rejected.\n";

    // Test 2: Wrong order type
    bool wrongType = playCardWithOrder(
        card, &alice, new Bomb()
    );

    if (wrongType ||
        hand->getSize() != initialHandSize ||
        deck.getSize() != initialDeckSize) {
        std::cerr << "FAIL: Wrong order type test.\n";
        return 1;
    }

    std::cout << "PASS: Wrong order type rejected.\n";

    // Test 3: Valid Diplomacy card
    bool success = playCardWithOrder(
        card, &alice, new Negotiate(&alice, &bob)
    );

    std::cout << "Helper returned: " << success << "\n";
    std::cout << "Hand size: " << hand->getSize() << "\n";
    std::cout << "Deck size: " << deck.getSize() << "\n";

    if (!success ||
        hand->getSize() != initialHandSize - 1 ||
        deck.getSize() != initialDeckSize + 1) {
        std::cerr << "FAIL: Valid card integration.\n";
        return 1;
    }

    // Only access the queued order after success.
    if (dynamic_cast<Negotiate*>(orders->getOrder(0)) == nullptr) {
        std::cerr << "FAIL: Incorrect queued order.\n";
        return 1;
    }

    std::cout << "PASS: Valid card creates Negotiate order.\n";
    std::cout << "PASS: Card returned to deck.\n";
    std::cout << "ALL CARD INTEGRATION CHECKS PASSED\n";

    return 0;
}
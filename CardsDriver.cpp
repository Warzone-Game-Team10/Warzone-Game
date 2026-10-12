
#include "Cards.h"
#include "Player.h"
#include "Orders.h"

#include <iostream>

int main() {
    std::cout << "===== WARZONE CARDS DRIVER ====="
              << std::endl;

    Deck deck;
    Hand hand(&deck);
    OrdersList orders;
    Player player;

    player.setCardHand(&hand);
    player.setOrdersList(&orders);

    // Test 1: Initial state
    std::cout << "\nTEST 1: Initial state" << std::endl;
    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    // Test 2: Draw five cards
    std::cout << "\nTEST 2: Drawing cards" << std::endl;

    for (int i = 0; i < 5; i++) {
        Card* card = deck.draw();

        if (card != nullptr) {
            std::cout << "Drew: " << *card << std::endl;
            hand.addCard(card);
        }
    }

    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    if (deck.getSize() == 0 && hand.getSize() == 5) {
        std::cout << "PASS: All five cards drawn." << std::endl;
    } else {
        std::cout << "FAIL: Unexpected card counts." << std::endl;
    }

    // Test 3: Empty deck
    std::cout << "\nTEST 3: Empty deck" << std::endl;

    Card* extraCard = deck.draw();

    if (extraCard == nullptr) {
        std::cout << "PASS: Empty deck handled."
                  << std::endl;
    } else {
        std::cout << "FAIL: Expected nullptr." << std::endl;
        hand.addCard(extraCard);
    }

    // Test 4: Copy constructor
    std::cout << "\nTEST 4: Copy constructor" << std::endl;

    Hand copiedHand(hand);

    if (copiedHand.getSize() == hand.getSize()) {
        std::cout << "PASS: Hand copied." << std::endl;
    } else {
        std::cout << "FAIL: Hand copy incorrect." << std::endl;
    }

    // Test 5: Assignment operator
    std::cout << "\nTEST 5: Assignment operator" << std::endl;

    Hand assignedHand(&deck);
    assignedHand = hand;

    if (assignedHand.getSize() == hand.getSize()) {
        std::cout << "PASS: Hand assigned." << std::endl;
    } else {
        std::cout << "FAIL: Hand assignment incorrect."
                  << std::endl;
    }

    // Test 6: Play all cards
    std::cout << "\nTEST 6: Playing cards" << std::endl;

    hand.playAll(&player);

    std::cout << deck << std::endl;
    std::cout << hand << std::endl;
    std::cout << orders << std::endl;

    if (deck.getSize() == 5 && hand.getSize() == 0) {
        std::cout << "PASS: All cards returned to deck."
                  << std::endl;
    } else {
        std::cout << "FAIL: Cards were not all returned."
                  << std::endl;
    }

    // Test 7: Reinforcement
    std::cout << "\nTEST 7: Reinforcement" << std::endl;

    std::cout << "Reinforcement pool: "
              << player.getReinforcementPool()
              << std::endl;

    if (player.getReinforcementPool() == 5) {
        std::cout << "PASS: Reinforcement applied."
                  << std::endl;
    } else {
        std::cout << "FAIL: Reinforcement incorrect."
                  << std::endl;
    }

    std::cout << "\n===== TEST COMPLETE ====="
              << std::endl;

    return 0;
}

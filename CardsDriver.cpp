
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

    // Test 1: Initial deck and hand
    std::cout << "\nTEST 1: Initial state" << std::endl;

    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    // Test 2: Draw every card
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
        std::cout << "PASS: All cards drawn." << std::endl;
    }
    else {
        std::cout << "FAIL: Drawing cards." << std::endl;
    }

    // Test 3: Drawing from an empty deck
    std::cout << "\nTEST 3: Empty deck" << std::endl;

    Card* extraCard = deck.draw();

    if (extraCard == nullptr) {
        std::cout << "PASS: Empty deck handled."
                  << std::endl;
    }
    else {
        std::cout << "FAIL: Expected nullptr."
                  << std::endl;
        hand.addCard(extraCard);
    }

    // Test 4: Copy constructor
    std::cout << "\nTEST 4: Copy constructor" << std::endl;

    Hand copiedHand(hand);

    std::cout << "Original: " << hand << std::endl;
    std::cout << "Copy: " << copiedHand << std::endl;

    // Test 5: Assignment operator
    std::cout << "\nTEST 5: Assignment operator" << std::endl;

    Hand assignedHand(&deck);
    assignedHand = hand;

    std::cout << "Assigned: "
              << assignedHand << std::endl;

    // Test 6: Play all cards
    std::cout << "\nTEST 6: Playing cards" << std::endl;

    hand.playAll(&player);

    std::cout << "\nAfter playing:" << std::endl;
    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    std::cout << "\nPlayer's orders:" << std::endl;
    std::cout << orders << std::endl;

    if (deck.getSize() == 5 && hand.getSize() == 0) {
        std::cout << "PASS: All cards returned to deck."
                  << std::endl;
    }
    else {
        std::cout << "FAIL: Not all cards were played."
                  << std::endl;
    }

    std::cout << "\n===== TEST COMPLETE ====="
              << std::endl;

    return 0;
}


#include "Cards.h"
#include "Player.h"
#include "Orders.h"

#include <iostream>

int main() {

    std::cout << "===== WARZONE CARDS DRIVER ====="
              << std::endl;

    // Test 1: Create Deck and Hand
    Deck deck;
    Hand hand(&deck);

    std::cout << "\nTEST 1: Initial state" << std::endl;
    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    // Test 2: Draw cards
    std::cout << "\nTEST 2: Drawing cards" << std::endl;

    for (int i = 0; i < 5; i++) {

        Card* card = deck.draw();

        if (card != nullptr) {
            std::cout << "Drew: " << *card << std::endl;
            hand.addCard(card);
        }
    }

    std::cout << "\nAfter drawing:" << std::endl;
    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    // Test 3: Draw from empty Deck
    std::cout << "\nTEST 3: Empty deck" << std::endl;

    Card* extraCard = deck.draw();

    if (extraCard == nullptr) {
        std::cout << "Empty deck handled correctly."
                  << std::endl;
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

    std::cout << "Assigned hand: "
              << assignedHand << std::endl;

    // Test 6: Play cards
    std::cout << "\nTEST 6: Playing cards" << std::endl;

    OrdersList orders;

    // Create a Player and give them an OrdersList.
    Player player;
    player.setOrdersList(&orders);

    hand.playAll(&player);

    std::cout << "\nAfter playing:" << std::endl;
    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    // Display orders created by played cards
    std::cout << "\nPlayer's orders:" << std::endl;
    std::cout << orders << std::endl;

    std::cout << "\n===== TEST COMPLETE ====="
              << std::endl;

    return 0;
}

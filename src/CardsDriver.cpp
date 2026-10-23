
#include "CardsDriver.h"
#include "Cards.h"
#include "Player.h"
#include "Orders.h"

#include <iostream>

void testCards() {
    std::cout << "===== WARZONE CARDS DRIVER =====" << std::endl;

    Deck deck;
    Player player;
    Hand hand(&deck);

    player.setCardHand(&hand);
    Hand* playerHand = player.getCardHand();

    // Test 1: Initial state
    std::cout << "\nTEST 1: Initial state" << std::endl;
    std::cout << deck << std::endl;
    std::cout << *playerHand << std::endl;

    // Test 2: Drawing cards
    std::cout << "\nTEST 2: Drawing cards" << std::endl;

    for (int i = 0; i < 5; i++) {
        Card* card = deck.draw();

        if (card != nullptr) {
            std::cout << "Drew: " << *card << std::endl;
            playerHand->addCard(card);
        }
    }

    std::cout << deck << std::endl;
    std::cout << *playerHand << std::endl;

    if (deck.getSize() == 0 && playerHand->getSize() == 5) {
        std::cout << "PASS: All cards drawn." << std::endl;
    } else {
        std::cout << "FAIL: Drawing cards." << std::endl;
    }

    // Test 3: Empty deck
    std::cout << "\nTEST 3: Empty deck" << std::endl;

    Card* extraCard = deck.draw();

    if (extraCard == nullptr) {
        std::cout << "PASS: Empty deck handled." << std::endl;
    } else {
        std::cout << "FAIL: Expected nullptr." << std::endl;
        playerHand->addCard(extraCard);
    }

    // Test 4: Copy constructor
    std::cout << "\nTEST 4: Copy constructor" << std::endl;

    Hand copiedHand(*playerHand);

    std::cout << "Original: " << *playerHand << std::endl;
    std::cout << "Copy: " << copiedHand << std::endl;

    // Test 5: Assignment operator
    std::cout << "\nTEST 5: Assignment operator" << std::endl;

    Hand assignedHand(&deck);
    assignedHand = *playerHand;

    std::cout << "Assigned: " << assignedHand << std::endl;

    // Test 6: Playing cards
    std::cout << "\nTEST 6: Playing cards" << std::endl;

    playerHand->playAll(&player);

    std::cout << "\nAfter playing:" << std::endl;
    std::cout << deck << std::endl;
    std::cout << *playerHand << std::endl;

    std::cout << "\nPlayer's orders:" << std::endl;
    std::cout << *player.getOrdersList() << std::endl;

    if (deck.getSize() == 5 && playerHand->getSize() == 0) {
        std::cout << "PASS: All cards returned to deck." << std::endl;
    } else {
        std::cout << "FAIL: Not all cards were played." << std::endl;
    }

    std::cout << "\n===== TEST COMPLETE =====" << std::endl;
}

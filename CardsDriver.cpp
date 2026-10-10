
#include "Cards.h"
#include <iostream>

int main() {

    std::cout << "===== WARZONE CARDS TEST ====="
              << std::endl;

    // Create a deck with 5 cards
    Deck deck;

    // Create a hand connected to deck
    Hand hand(&deck);

    std::cout << "\nBEFORE DRAWING:" << std::endl;
    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    // Draw all 5 cards
    for (int i = 0; i < 5; i++) {

        Card* card = deck.draw();

        if (card != nullptr) {

            std::cout << "Drew: "
                      << *card << std::endl;

            hand.addCard(card);
        }
    }

    std::cout << "\nAFTER DRAWING:" << std::endl;
    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    // Try drawing from an empty deck
    Card* extraCard = deck.draw();

    if (extraCard == nullptr) {
        std::cout << "Deck is empty!"
                  << std::endl;
    }

    // Play all cards
    std::cout << "\nPLAYING CARDS:"
              << std::endl;

    hand.playAll();

    std::cout << "\nAFTER PLAYING:" << std::endl;
    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    std::cout << "\n===== TEST COMPLETE ====="
              << std::endl;

    return 0;
}

#include "Cards.h"
#include <iostream>

int main() {

    std::cout << "================================"
              << std::endl;
    std::cout << " WARZONE CARDS DRIVER"
              << std::endl;
    std::cout << "================================"
              << std::endl;

    // ======================================
    // TEST 1: CREATE DECK AND HAND
    // ======================================

    Deck deck;
    Hand hand(&deck);

    std::cout << "\nTEST 1: Initial state"
              << std::endl;

    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    // ======================================
    // TEST 2: DRAW CARDS
    // ======================================

    std::cout << "\nTEST 2: Drawing cards"
              << std::endl;

    for (int i = 0; i < 5; i++) {

        Card* card = deck.draw();

        if (card != nullptr) {

            std::cout << "Drew: "
                      << *card << std::endl;

            hand.addCard(card);
        }
    }

    std::cout << "\nAfter drawing:"
              << std::endl;

    std::cout << deck << std::endl;
    std::cout << hand << std::endl;

    // ======================================
    // TEST 3: EMPTY DECK
    // ======================================

    std::cout << "\nTEST 3: Empty deck"
              << std::endl;

    Card* extraCard = deck.draw();

    if (extraCard == nullptr) {
        std::cout << "Empty deck handled correctly."
                  << std::endl;
    }

    // ======================================
    // TEST 4: COPY CONSTRUCTOR
    // ======================================

    std::cout << "\nTEST 4: Copy constructor"
              << std::endl;

    Hand copiedHand(hand);

    std::cout << "Original: "
              << hand << std::endl;

    std::cout << "Copy: "
              << copiedHand << std::endl;

    // ======================================
    // TEST 5: ASSIGNMENT OPERATOR
    // ======================================

    std::cout << "\nTEST 5: Assignment operator"
              << std::endl;

    Hand assignedHand(&deck);

    assignedHand = hand;

    std::cout << "Assigned hand: "
              << assignedHand << std::endl;

    // ======================================
    // TODO TEST 6: PLAY CARDS
    // ======================================

    // Once Player and Orders are ready:

    // ======================================
    // TODO TEST MEMORY MANAGEMENT
    // ======================================


    std::cout << "\n================================"
              << std::endl;
    std::cout << " STANDALONE TESTS COMPLETE"
              << std::endl;
    std::cout << "================================"
              << std::endl;

    return 0;
}

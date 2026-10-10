
#pragma once

#include <iostream>
#include <vector>
#include <string>

// ==========================================
// CARDS DECK/HAND
// ==========================================

// The five types of Warzone cards
enum class CardType {
    BOMB,
    REINFORCEMENT,
    BLOCKADE,
    AIRLIFT,
    DIPLOMACY
};

// Forward declarations
// These tell C++ that the classes exist.
class Player;
class Order;
class OrdersList;
class Deck;
class Hand;

// ==========================================
// CARD CLASS
// Represents one Warzone card.
// ==========================================

class Card {
private:
    CardType type;

public:
    // Constructors
    Card();
    Card(CardType type);
    Card(const Card& other);

    // Destructor
    ~Card();

    // Assignment operator
    Card& operator=(const Card& other);

    // Getters
    CardType getType() const;
    std::string getTypeName() const;

    // TODO: Connect to Player and Orders.
    // This will eventually create an Order,
    // remove the card from Hand, and return
    // it to Deck.
    void play(Player* player, Hand* hand, Deck* deck);

    // Stream insertion operator
    friend std::ostream& operator<<(
        std::ostream& out, const Card& card);
};

// ==========================================
// DECK CLASS
// Stores cards available to draw.
// ==========================================

class Deck {
private:
    std::vector<Card*> cards;

public:
    // Constructors
    Deck();
    Deck(const Deck& other);

    // Destructor
    ~Deck();

    // Assignment operator
    Deck& operator=(const Deck& other);

    // Draw a random card
    Card* draw();

    // Return a card to the deck
    void returnCard(Card* card);

    // Get number of cards
    int getSize() const;

    // Stream insertion operator
    friend std::ostream& operator<<(
        std::ostream& out, const Deck& deck);
};

// ==========================================
// HAND CLASS (TODO)
// Stores cards held by a player.
// ==========================================

class Hand {
private:
    std::vector<Card*> handCards;

    // Pointer to the deck.
    // Hand does NOT own the Deck.
    Deck* deck;

public:
    // Constructors
    Hand(Deck* deck);
    Hand(const Hand& other);

    // Destructor
    ~Hand();

    // Assignment operator
    Hand& operator=(const Hand& other);

    // Add a card to the hand
    void addCard(Card* card);

    // Remove a card from the hand
    bool removeCard(Card* card);

    // Check whether a card is in the hand
    bool hasCard(Card* card) const;

    // TODO: Play cards through Player/Orders
    void playAll(Player* player);

    // Get number of cards
    int getSize() const;

    // Stream insertion operator
    friend std::ostream& operator<<(
        std::ostream& out, const Hand& hand);
};


#pragma once

#include <iostream>
#include <string>
#include <vector>

// The five card types required for Warzone.
enum class CardType {
    BOMB,
    REINFORCEMENT,
    BLOCKADE,
    AIRLIFT,
    DIPLOMACY
};

class Player;
class Hand;
class Deck;

// Represents one Warzone card.
class Card {
private:
    CardType type;

public:
    Card();
    explicit Card(CardType type);
    Card(const Card& other);
    ~Card();

    Card& operator=(const Card& other);

    CardType getType() const;
    std::string getTypeName() const;

    // Plays the card and transfers it from Hand to Deck.
    void play(Player* player, Hand* hand, Deck* deck);

    friend std::ostream& operator<<(std::ostream& out,
                                    const Card& card);
};

// Owns all Card pointers currently stored in the Deck.
class Deck {
private:
    std::vector<Card*> cards;

public:
    Deck();
    Deck(const Deck& other);
    ~Deck();

    Deck& operator=(const Deck& other);

    // Draws a random card and transfers ownership to the caller.
    Card* draw();

    // Takes ownership of a card returned to the Deck.
    void returnCard(Card* card);

    int getSize() const;

    friend std::ostream& operator<<(std::ostream& out,
                                    const Deck& deck);
};

// Owns cards in the Hand.
// Does NOT own the Deck pointer.
class Hand {
private:
    std::vector<Card*> handCards;
    Deck* deck;

public:
    explicit Hand(Deck* deck);
    Hand(const Hand& other);
    ~Hand();

    Hand& operator=(const Hand& other);

    // Hand takes ownership of the added Card.
    void addCard(Card* card);

    // Removes the Card without deleting it.
    bool removeCard(Card* card);

    bool hasCard(Card* card) const;

    void playAll(Player* player);

    int getSize() const;
    Deck* getDeck() const;

    friend std::ostream& operator<<(std::ostream& out,
                                    const Hand& hand);
};


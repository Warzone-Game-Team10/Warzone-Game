#pragma once

#include <iostream>
#include <vector>
#include <string>

// Five types of Warzone cards
enum class CardType {
    BOMB,
    REINFORCEMENT,
    BLOCKADE,
    AIRLIFT,
    DIPLOMACY
};

class Player;
class Deck;
class Hand;

// Represents one card
class Card {
private:
    CardType type;

public:
    Card();
    Card(CardType type);
    Card(const Card& other);
    ~Card();

    Card& operator=(const Card& other);

    CardType getType() const;
    std::string getTypeName() const;

    // Creates an order and returns the played card to the deck
    void play(Player* player, Hand* hand, Deck* deck);

    friend std::ostream& operator<<(std::ostream& out,
                                    const Card& card);
};

// Stores cards available to draw
class Deck {
private:
    std::vector<Card*> cards;

public:
    Deck();
    Deck(const Deck& other);
    ~Deck();

    Deck& operator=(const Deck& other);

    Card* draw();
    void returnCard(Card* card);
    int getSize() const;

    friend std::ostream& operator<<(std::ostream& out,
                                    const Deck& deck);
};

// Stores cards belonging to a player
class Hand {
private:
    std::vector<Card*> handCards;
    Deck* deck; // Hand does not own the Deck

public:
    Hand(Deck* deck);
    Hand(const Hand& other);
    ~Hand();

    Hand& operator=(const Hand& other);

    void addCard(Card* card);
    bool removeCard(Card* card);
    bool hasCard(Card* card) const;

    void playAll(Player* player);

    int getSize() const;
    Deck* getDeck() const;

    friend std::ostream& operator<<(std::ostream& out,
                                    const Hand& hand);
};
// Remove once the Player teammate changes Cards* to Hand*.
using Cards = Hand;


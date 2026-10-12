#pragma once

#include <iostream>
#include <string>
#include <vector>

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

    void play(Player* player, Hand* hand, Deck* deck);

    friend std::ostream& operator<<(std::ostream& out,
                                    const Card& card);
};

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

class Hand {
private:
    std::vector<Card*> handCards;
    Deck* deck; // Non-owning pointer

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

// Compatibility with the Player class
using Cards = Hand;

#include "Cards.h"
#include "Orders.h"
#include "Player.h"

#include <cstdlib>
#include <ctime>

// Card constructors and operators

Card::Card() : type(CardType::BOMB) {
}

Card::Card(CardType type) : type(type) {
}

Card::Card(const Card& other) : type(other.type) {
}

Card::~Card() {
}

Card& Card::operator=(const Card& other) {
    if (this != &other) {
        type = other.type;
    }
    return *this;
}

CardType Card::getType() const {
    return type;
}

std::string Card::getTypeName() const {
    switch (type) {
        case CardType::BOMB:
            return "Bomb";
        case CardType::REINFORCEMENT:
            return "Reinforcement";
        case CardType::BLOCKADE:
            return "Blockade";
        case CardType::AIRLIFT:
            return "Airlift";
        case CardType::DIPLOMACY:
            return "Diplomacy";
    }

    return "Unknown";
}

// Play a card, then return it to the deck.

void Card::play(Player* player, Hand* hand, Deck* deck) {
    if (player == nullptr || hand == nullptr || deck == nullptr) {
        std::cout << "Cannot play card: missing object."
                  << std::endl;
        return;
    }

    if (hand->getDeck() != deck) {
        std::cout << "Card is being returned to the wrong deck."
                  << std::endl;
        return;
    }

    if (!hand->hasCard(this)) {
        std::cout << "Card is not in the hand."
                  << std::endl;
        return;
    }

    OrdersList* orders = player->getOrdersList();

    if (orders == nullptr) {
        std::cout << "Player has no OrdersList."
                  << std::endl;
        return;
    }

    Order* newOrder = nullptr;

    switch (type) {
        case CardType::BOMB:
            newOrder = new Bomb();
            break;

        case CardType::BLOCKADE:
            newOrder = new Blockade();
            break;

        case CardType::AIRLIFT:
            newOrder = new Airlift();
            break;

        case CardType::DIPLOMACY:
            newOrder = new Negotiate();
            break;

        case CardType::REINFORCEMENT:
            // Confirm the +5 rule with the assignment instructions.
            player->addReinforcements(5);
            break;
    }

    if (!hand->removeCard(this)) {
        delete newOrder;
        std::cout << "Could not remove card from hand."
                  << std::endl;
        return;
    }

    if (newOrder != nullptr) {
        orders->addOrder(newOrder);
    }

    deck->returnCard(this);

    std::cout << getTypeName() << " card played."
              << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Card& card) {
    out << card.getTypeName();
    return out;
}

// Deck

Deck::Deck() {
    cards.push_back(new Card(CardType::BOMB));
    cards.push_back(new Card(CardType::REINFORCEMENT));
    cards.push_back(new Card(CardType::BLOCKADE));
    cards.push_back(new Card(CardType::AIRLIFT));
    cards.push_back(new Card(CardType::DIPLOMACY));

    static bool seeded = false;

    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }
}

Deck::Deck(const Deck& other) {
    for (Card* card : other.cards) {
        cards.push_back(new Card(*card));
    }
}

Deck::~Deck() {
    for (Card* card : cards) {
        delete card;
    }
}

Deck& Deck::operator=(const Deck& other) {
    if (this != &other) {
        for (Card* card : cards) {
            delete card;
        }

        cards.clear();

        for (Card* card : other.cards) {
            cards.push_back(new Card(*card));
        }
    }

    return *this;
}

Card* Deck::draw() {
    if (cards.empty()) {
        std::cout << "Deck is empty!" << std::endl;
        return nullptr;
    }

    int index = std::rand() % cards.size();
    Card* drawnCard = cards[index];

    cards.erase(cards.begin() + index);

    return drawnCard;
}

void Deck::returnCard(Card* card) {
    if (card == nullptr) {
        return;
    }

    for (Card* existing : cards) {
        if (existing == card) {
            return;
        }
    }

    cards.push_back(card);
}

int Deck::getSize() const {
    return static_cast<int>(cards.size());
}

std::ostream& operator<<(std::ostream& out, const Deck& deck) {
    out << "Deck contains " << deck.getSize() << " cards.";
    return out;
}

// Hand

Hand::Hand(Deck* deck) : deck(deck) {
}

Hand::Hand(const Hand& other) : deck(other.deck) {
    for (Card* card : other.handCards) {
        handCards.push_back(new Card(*card));
    }
}

Hand::~Hand() {
    for (Card* card : handCards) {
        delete card;
    }
}

Hand& Hand::operator=(const Hand& other) {
    if (this != &other) {
        for (Card* card : handCards) {
            delete card;
        }

        handCards.clear();
        deck = other.deck;

        for (Card* card : other.handCards) {
            handCards.push_back(new Card(*card));
        }
    }

    return *this;
}

void Hand::addCard(Card* card) {
    if (card != nullptr && !hasCard(card)) {
        handCards.push_back(card);
    }
}

bool Hand::hasCard(Card* card) const {
    for (Card* existing : handCards) {
        if (existing == card) {
            return true;
        }
    }

    return false;
}

bool Hand::removeCard(Card* card) {
    for (int i = 0; i < static_cast<int>(handCards.size()); i++) {
        if (handCards[i] == card) {
            handCards.erase(handCards.begin() + i);
            return true;
        }
    }

    return false;
}

void Hand::playAll(Player* player) {
    if (player == nullptr || deck == nullptr) {
        std::cout << "Cannot play cards." << std::endl;
        return;
    }

    int i = 0;

    while (i < static_cast<int>(handCards.size())) {
        Card* card = handCards[i];
        int oldSize = getSize();

        card->play(player, this, deck);

        if (getSize() == oldSize) {
            i++;
        }
    }
}

int Hand::getSize() const {
    return static_cast<int>(handCards.size());
}

Deck* Hand::getDeck() const {
    return deck;
}

std::ostream& operator<<(std::ostream& out, const Hand& hand) {
    out << "Hand contains " << hand.getSize() << " cards.";
    return out;
}

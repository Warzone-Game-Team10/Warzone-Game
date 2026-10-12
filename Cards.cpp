
#include "Cards.h"
#include "Orders.h"
#include "Player.h"

#include <cstdlib>
#include <ctime>

// CARD

Card::Card() {
    type = CardType::BOMB;
}

Card::Card(CardType type) {
    this->type = type;
}

Card::Card(const Card& other) {
    type = other.type;
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
    if (type == CardType::BOMB) {
        return "Bomb";
    }
    else if (type == CardType::REINFORCEMENT) {
        return "Reinforcement";
    }
    else if (type == CardType::BLOCKADE) {
        return "Blockade";
    }
    else if (type == CardType::AIRLIFT) {
        return "Airlift";
    }
    else if (type == CardType::DIPLOMACY) {
        return "Diplomacy";
    }

    return "Unknown";
}

// Play a card and add its order to the player's list
void Card::play(Player* player, Hand* hand, Deck* deck) {

    if (player == nullptr || hand == nullptr || deck == nullptr) {
        std::cout << "Cannot play card: missing object."
                  << std::endl;
        return;
    }

    if (hand->getDeck() != deck) {
        std::cout << "Cannot play card: incorrect deck."
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

    if (type == CardType::BOMB) {
        newOrder = new Bomb();
    }
    else if (type == CardType::REINFORCEMENT) {
        // Proposed mapping: Reinforcement creates a Deploy order.
        // Confirm with the team or TA.
        newOrder = new Deploy();
    }
    else if (type == CardType::BLOCKADE) {
        newOrder = new Blockade();
    }
    else if (type == CardType::AIRLIFT) {
        newOrder = new Airlift();
    }
    else if (type == CardType::DIPLOMACY) {
        newOrder = new Negotiate();
    }

    if (newOrder == nullptr) {
        return;
    }

    // Remove the card before transferring it to the deck
    if (!hand->removeCard(this)) {
        delete newOrder;
        std::cout << "Could not remove card from hand."
                  << std::endl;
        return;
    }

    // OrdersList takes ownership of the new order
    orders->addOrder(newOrder);

    // Deck takes ownership of the played card
    deck->returnCard(this);

    std::cout << getTypeName()
              << " card played." << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Card& card) {
    out << card.getTypeName();
    return out;
}

// DECK

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

    cards.clear();
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
    out << "Deck contains "
        << deck.getSize()
        << " cards.";

    return out;
}

// HAND

Hand::Hand(Deck* deck) {
    this->deck = deck;
}

Hand::Hand(const Hand& other) {
    deck = other.deck;

    for (Card* card : other.handCards) {
        handCards.push_back(new Card(*card));
    }
}

Hand::~Hand() {
    for (Card* card : handCards) {
        delete card;
    }

    handCards.clear();
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
    if (card == nullptr || hasCard(card)) {
        return;
    }

    handCards.push_back(card);
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

// Play every card in the hand
void Hand::playAll(Player* player) {
    if (player == nullptr || deck == nullptr) {
        std::cout << "Cannot play cards."
                  << std::endl;
        return;
    }

    int i = 0;

    while (i < static_cast<int>(handCards.size())) {
        Card* card = handCards[i];
        int oldSize = getSize();

        card->play(player, this, deck);

        // A successfully played card is removed from the hand.
        // If it wasn't removed, move to the next card.
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
    out << "Hand contains "
        << hand.getSize()
        << " cards.";

    return out;
}


#include "Cards.h"
#include "Orders.h"
#include "Player.h"

#include <cstdlib>
#include <ctime>

// ==================================================
// CARD
// ==================================================

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

// Play a card and add its order to the player's OrdersList.
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

    if (player->getCardHand() != hand) {
        std::cout << "Cannot play card: incorrect player hand."
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
        // Temporary mapping. Confirm with TA.
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
        std::cout << "Unknown card type." << std::endl;
        return;
    }

    // Remove card from Hand without deleting it.
    if (!hand->removeCard(this)) {
        delete newOrder;
        return;
    }

    // OrdersList now owns the new Order.
    orders->addOrder(newOrder);

    // Deck now owns the played Card.
    deck->returnCard(this);

    std::cout << getTypeName()
              << " card played." << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Card& card) {
    out << card.getTypeName();
    return out;
}

// ==================================================
// DECK
// ==================================================

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

// Deep copy constructor.
Deck::Deck(const Deck& other) {
    for (Card* card : other.cards) {
        cards.push_back(new Card(*card));
    }
}

// Destructor: delete cards still in the Deck.
Deck::~Deck() {
    for (Card* card : cards) {
        delete card;
    }

    cards.clear();
}

// Deep copy assignment operator.
Deck& Deck::operator=(const Deck& other) {
    if (this != &other) {

        // Copy first so the original cards remain
        // unchanged if copying fails.
        Deck copy(other);

        cards.swap(copy.cards);
    }

    return *this;
}

// Draw a random card and remove it from the Deck.
Card* Deck::draw() {
    if (cards.empty()) {
        std::cout << "Deck is empty!" << std::endl;
        return nullptr;
    }

    int index = std::rand() % cards.size();

    Card* drawnCard = cards[index];

    cards.erase(cards.begin() + index);

    // Hand will own this card once it is added.
    return drawnCard;
}

// Return a played card to the Deck.
void Deck::returnCard(Card* card) {
    if (card == nullptr) {
        return;
    }

    // Avoid storing the same pointer twice.
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

// ==================================================
// HAND
// ==================================================

Hand::Hand(Deck* deck) {
    this->deck = deck;
}

// Deep copy constructor.
Hand::Hand(const Hand& other) {
    deck = other.deck;

    for (Card* card : other.handCards) {
        handCards.push_back(new Card(*card));
    }
}

// Destructor: delete cards still in the Hand.
Hand::~Hand() {
    for (Card* card : handCards) {
        delete card;
    }

    handCards.clear();
}

// Deep copy assignment operator.
Hand& Hand::operator=(const Hand& other) {
    if (this != &other) {

        // Make a copy before deleting existing cards.
        Hand copy(other);

        handCards.swap(copy.handCards);
        deck = other.deck;
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

// Remove card without deleting it.
// The Deck will take ownership when the card is played.
bool Hand::removeCard(Card* card) {
    for (int i = 0; i < static_cast<int>(handCards.size()); i++) {
        if (handCards[i] == card) {
            handCards.erase(handCards.begin() + i);
            return true;
        }
    }

    return false;
}

// Play every card in the Hand.
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

        // If card was not removed, move to next card.
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

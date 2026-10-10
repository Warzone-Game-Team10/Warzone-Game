
#include "Cards.h"

#include <cstdlib>
#include <ctime>

// ==========================================
// CARD CLASS IMPLEMENTATION
// ==========================================

// Default constructor
Card::Card() {
    type = CardType::BOMB;
}

// Constructor with card type
Card::Card(CardType type) {
    this->type = type;
}

// Copy constructor
Card::Card(const Card& other) {
    type = other.type;
}

// Destructor
Card::~Card() {
}

// Assignment operator
Card& Card::operator=(const Card& other) {

    if (this != &other) {
        type = other.type;
    }

    return *this;
}

// Return the card type
CardType Card::getType() const {
    return type;
}

// Return the name of the card
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

// ==========================================
// CARD PLAY METHOD
// ==========================================

void Card::play(Player* player, Hand* hand, Deck* deck) {

    // Check that pointers are valid
    if (player == nullptr ||
        hand == nullptr ||
        deck == nullptr) {

        std::cout << "Cannot play card: missing object."
                  << std::endl;
        return;
    }

    // Make sure this card is in the hand
    if (!hand->hasCard(this)) {
        std::cout << "Card is not in the hand."
                  << std::endl;
        return;
    }

    std::cout << "Playing " << getTypeName()
              << " card." << std::endl;

    // ======================================
    // TODO 1: CREATE THE CORRECT ORDER
    // ======================================

    // Include Player.h and Orders.h later.


    // ======================================
    // TODO 2: ADD ORDER TO PLAYER
    // ======================================

    // ======================================
    // TODO 3: REMOVE CARD FROM HAND
    // ======================================


    // ======================================
    // TODO 4: RETURN CARD TO DECK
    // ======================================


}

// Print a Card
std::ostream& operator<<(std::ostream& out,
                         const Card& card) {

    out << card.getTypeName();
    return out;
}

// ==========================================
// DECK CLASS IMPLEMENTATION
// ==========================================

// Default constructor
Deck::Deck() {

    // Add one card of each type
    cards.push_back(new Card(CardType::BOMB));
    cards.push_back(new Card(CardType::REINFORCEMENT));
    cards.push_back(new Card(CardType::BLOCKADE));
    cards.push_back(new Card(CardType::AIRLIFT));
    cards.push_back(new Card(CardType::DIPLOMACY));

    // Seed random generator once
    static bool seeded = false;

    if (!seeded) {
        std::srand(
            static_cast<unsigned int>(std::time(0))
        );
        seeded = true;
    }
}

// Copy constructor
Deck::Deck(const Deck& other) {

    for (Card* card : other.cards) {
        cards.push_back(new Card(*card));
    }
}

// Destructor
Deck::~Deck() {

    for (Card* card : cards) {
        delete card;
    }

    cards.clear();
}

// Assignment operator
Deck& Deck::operator=(const Deck& other) {

    if (this != &other) {

        // Delete old cards
        for (Card* card : cards) {
            delete card;
        }

        cards.clear();

        // Copy cards from other deck
        for (Card* card : other.cards) {
            cards.push_back(new Card(*card));
        }
    }

    return *this;
}

// Draw a random card
Card* Deck::draw() {

    if (cards.empty()) {
        std::cout << "Deck is empty!"
                  << std::endl;
        return nullptr;
    }

    // Pick random index
    int index = std::rand() % cards.size();

    // Save the selected card
    Card* drawnCard = cards[index];

    // Remove it from the deck
    cards.erase(cards.begin() + index);

    return drawnCard;
}

// Return a card to the deck
void Deck::returnCard(Card* card) {

    if (card != nullptr) {
        cards.push_back(card);
    }
}

// Return number of cards
int Deck::getSize() const {
    return static_cast<int>(cards.size());
}

// Print Deck
std::ostream& operator<<(std::ostream& out,
                         const Deck& deck) {

    out << "Deck contains "
        << deck.getSize()
        << " cards.";

    return out;
}

// ==========================================
// HAND CLASS IMPLEMENTATION
// ==========================================

// Constructor
Hand::Hand(Deck* deck) {
    this->deck = deck;
}

// Copy constructor
Hand::Hand(const Hand& other) {

    deck = other.deck;

    for (Card* card : other.handCards) {
        handCards.push_back(new Card(*card));
    }
}

// Destructor
Hand::~Hand() {

    // Delete cards still owned by Hand
    for (Card* card : handCards) {
        delete card;
    }

    handCards.clear();
}

// Assignment operator
Hand& Hand::operator=(const Hand& other) {

    if (this != &other) {

        // Delete old cards
        for (Card* card : handCards) {
            delete card;
        }

        handCards.clear();

        // Copy deck pointer (not ownership)
        deck = other.deck;

        // Make copies of cards
        for (Card* card : other.handCards) {
            handCards.push_back(new Card(*card));
        }
    }

    return *this;
}

// Add a card to Hand
void Hand::addCard(Card* card) {

    if (card != nullptr) {
        handCards.push_back(card);
    }
}

// Check whether a card is in Hand
bool Hand::hasCard(Card* card) const {

    for (Card* currentCard : handCards) {

        if (currentCard == card) {
            return true;
        }
    }

    return false;
}

// Remove a card from Hand
bool Hand::removeCard(Card* card) {

    for (int i = 0;
         i < static_cast<int>(handCards.size());
         i++) {

        if (handCards[i] == card) {

            // Remove pointer, not the object
            handCards.erase(handCards.begin() + i);

            return true;
        }
    }

    return false;
}

// ==========================================
// PLAY ALL CARDS
// ==========================================

void Hand::playAll(Player* player) {

    if (player == nullptr || deck == nullptr) {
        std::cout << "Cannot play cards."
                  << std::endl;
        return;
    }

    // TODO: Card::play() to be implemented
   
}

// Return number of cards
int Hand::getSize() const {
    return static_cast<int>(handCards.size());
}

// Print Hand
std::ostream& operator<<(std::ostream& out,
                         const Hand& hand) {

    out << "Hand contains "
        << hand.getSize()
        << " cards.";

    return out;
}

#pragma once

#include "Cards.h"
#include "Orders.h"
#include "Player.h"

#include <memory>

// Plays a card using an already configured order.
// OrdersList owns the order after successful insertion.
// Deck owns the card after it is returned.

inline bool playCardWithOrder(
    Card* card,
    Player* player,
    Order* proposedOrder
) {
    std::unique_ptr<Order> order(proposedOrder);

    if (!card || !player || !order) {
        return false;
    }

    Hand* hand = player->getCardHand();
    OrdersList* orders = player->getOrdersList();

    if (!hand || !orders || !hand->getDeck()) {
        return false;
    }

    if (!hand->hasCard(card)) {
        return false;
    }

    bool correctType = false;

    switch (card->getType()) {
        case CardType::BOMB:
            correctType =
                (dynamic_cast<Bomb*>(order.get()) != nullptr && dynamic_cast<Bomb*>(order.get())->getIssuer() == player);
            break;

        case CardType::BLOCKADE:
            correctType =
                (dynamic_cast<Blockade*>(order.get()) != nullptr && dynamic_cast<Blockade*>(order.get())->getIssuer() == player);
            break;

        case CardType::AIRLIFT:
            correctType =
                (dynamic_cast<Airlift*>(order.get()) != nullptr && dynamic_cast<Airlift*>(order.get())->getIssuer() == player);
            break;

        case CardType::DIPLOMACY:
            correctType =
                (dynamic_cast<Negotiate*>(order.get()) != nullptr && dynamic_cast<Negotiate*>(order.get())->getIssuer() == player);
            break;

        case CardType::REINFORCEMENT:
            // Reinforcement behavior requires confirmation.
            return false;

        default:
            return false;
    }

    // Reject mismatched or invalid orders.
    if (!correctType || !order->validate()) {
        return false;
    }

    // Transfer order ownership to OrdersList.
    orders->addOrder(order.get());
    order.release();

    // Transfer card ownership back to Deck.
    hand->removeCard(card);
    hand->getDeck()->returnCard(card);

    return true;
}
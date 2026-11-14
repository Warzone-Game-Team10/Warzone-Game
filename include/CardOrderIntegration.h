#pragma once

#include "Cards.h"
#include "Orders.h"
#include "Player.h"

#include <memory>

// Plays a card using an already configured order.
// OrdersList owns the order after successful insertion.
// Deck owns the card after it is returned.

bool playCardWithOrder(Card* card, Player* player, Order* proposedOrder);

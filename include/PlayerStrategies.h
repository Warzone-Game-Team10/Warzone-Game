#pragma once

#include <stdio.h>
#include <vector>
#include <string>
#include "Map.h"

using namespace std;

// Forward declarations to avoid circular dependencies
class Player;
class Card;
class Order;

class PlayerStrategy {
    
    public: 
        //attributes
        Player* player;
        // True once the strategy has nothing more to do this turn.
        bool finishedIssuing;

        //functions
        PlayerStrategy(Player* player);
        PlayerStrategy(const PlayerStrategy& other);
        //clone is needed to create a copy of the strategy when assigning a new strategy to a player
        virtual PlayerStrategy* clone() const = 0;
        PlayerStrategy& operator=(const PlayerStrategy& other);
        virtual ~PlayerStrategy();
        
        //helpers
        virtual void beginIssuing();
        void setPlayer(Player* player);
        // The territories that touch one of the player's territories but are not the player's own.
        vector<Territory*> findAdjacentEnemyTerritories() const;
        // The territories owned by the player.
        vector<Territory*> findOwnedTerritories() const;

        // Pure virtual functions that must be implemented by derived classes
        virtual bool issueOrder() = 0;
        virtual vector<Territory*> toAttack() = 0;
        virtual vector<Territory*> toDefend() = 0;

        friend ostream& operator<<(ostream& out, const PlayerStrategy& ps);
};

class HumanPlayerStrategy : public PlayerStrategy {
    public:

        //atributes
        int committedDeployArmies;                 // armies already promised to Deploy orders
        std::unordered_map<int, int> deployedThisTurn; // territory ID -> armies deployed there this turn
        std::unordered_map<int, int> advancedThisTurn; // territory ID -> armies ordered

        HumanPlayerStrategy(Player* player);
        HumanPlayerStrategy(const HumanPlayerStrategy& other);
        PlayerStrategy* clone() const override;
        HumanPlayerStrategy& operator=(const HumanPlayerStrategy& other);
        ~HumanPlayerStrategy();

        //helper functions
        virtual void beginIssuing() override;
        bool issueDeployOrder(int armiesLeftToDeploy);
        bool issueMenuOrder();
        bool issueAdvanceOrder();
        bool issueCardOrder();
        Order* buildCardOrder(Card* card, int& movedSourceId, int& movedArmies);
        int getArmiesAvailableToMove(Territory* territory) const;


        bool issueOrder() override;
        vector<Territory*> toAttack() override;
        vector<Territory*> toDefend() override;

        
        friend ostream& operator<<(ostream& out, const HumanPlayerStrategy& hps);
}; 

class CheaterPlayerStrategy : public PlayerStrategy {
    public:
        CheaterPlayerStrategy(Player* player);
        CheaterPlayerStrategy(const CheaterPlayerStrategy& other);
        CheaterPlayerStrategy& operator=(const CheaterPlayerStrategy& other);
        PlayerStrategy* clone() const override;
        ~CheaterPlayerStrategy();
        bool issueOrder() override;
        vector<Territory*> toAttack() override;
        vector<Territory*> toDefend() override;

        
        friend ostream& operator<<(ostream& out, const CheaterPlayerStrategy& cps);
};

class AggressivePlayerStrategy : public PlayerStrategy {
    public:
        AggressivePlayerStrategy(Player* player);
        AggressivePlayerStrategy(const AggressivePlayerStrategy& other);
        AggressivePlayerStrategy& operator=(const AggressivePlayerStrategy& other);
        PlayerStrategy* clone() const override;
        ~AggressivePlayerStrategy();
        bool issueOrder() override;
        vector<Territory*> toAttack() override;
        vector<Territory*> toDefend() override;

        
        friend ostream& operator<<(ostream& out, const AggressivePlayerStrategy& aps);
};

class BenevolentPlayerStrategy : public PlayerStrategy {
    public:
        BenevolentPlayerStrategy(Player* player);
        BenevolentPlayerStrategy(const BenevolentPlayerStrategy& other);
        BenevolentPlayerStrategy& operator=(const BenevolentPlayerStrategy& other);
        PlayerStrategy* clone() const override;
        ~BenevolentPlayerStrategy();
        bool issueOrder() override;
        vector<Territory*> toAttack() override;
        vector<Territory*> toDefend() override;

        
        friend ostream& operator<<(ostream& out, const BenevolentPlayerStrategy& bps);
};

class NeutralPlayerStrategy : public PlayerStrategy {
    public:
        NeutralPlayerStrategy(Player* player);
        NeutralPlayerStrategy(const NeutralPlayerStrategy& other);
        NeutralPlayerStrategy& operator=(const NeutralPlayerStrategy& other);
        PlayerStrategy* clone() const override;
        ~NeutralPlayerStrategy();
        bool issueOrder() override;
        vector<Territory*> toAttack() override;
        vector<Territory*> toDefend() override;

        friend ostream& operator<<(ostream& out, const NeutralPlayerStrategy& nps);
};
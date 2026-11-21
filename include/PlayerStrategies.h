#pragma once

#include <stdio.h>
#include <vector>
#include <string>
#include "Map.h"

using namespace std;

class PlayerStrategy {
    
    public: 
        Player* p;
        PlayerStrategy(Player* player);
        PlayerStrategy(const PlayerStrategy& other);
        PlayerStrategy& operator=(const PlayerStrategy& other);
        virtual ~PlayerStrategy();
        virtual void issueOrder() = 0;
        virtual vector<Territory*> toAttack() = 0;
        virtual vector<Territory*> toDefend() = 0;

        //clone is needed to create a copy of the strategy when assigning a new strategy to a player
        virtual PlayerStrategy* clone() const = 0;
        friend ostream& operator<<(ostream& out, const PlayerStrategy& ps);
};

class HumanPlayerStrategy : public PlayerStrategy {
    public:
        HumanPlayerStrategy(Player* player);
        HumanPlayerStrategy(const HumanPlayerStrategy& other);
        HumanPlayerStrategy& operator=(const HumanPlayerStrategy& other);
        ~HumanPlayerStrategy();
        void issueOrder() override;
        vector<Territory*> toAttack() override;
        vector<Territory*> toDefend() override;

        PlayerStrategy* clone() const override;
        friend ostream& operator<<(ostream& out, const HumanPlayerStrategy& hps);
};

class CheaterPlayerStrategy : public PlayerStrategy {
    public:
        CheaterPlayerStrategy(Player* player);
        CheaterPlayerStrategy(const CheaterPlayerStrategy& other);
        CheaterPlayerStrategy& operator=(const CheaterPlayerStrategy& other);
        ~CheaterPlayerStrategy();
        void issueOrder() override;
        vector<Territory*> toAttack() override;
        vector<Territory*> toDefend() override;

        PlayerStrategy* clone() const override;
        friend ostream& operator<<(ostream& out, const CheaterPlayerStrategy& cps);
};

class AggressivePlayerStrategy : public PlayerStrategy {
    public:
        AggressivePlayerStrategy(Player* player);
        AggressivePlayerStrategy(const AggressivePlayerStrategy& other);
        AggressivePlayerStrategy& operator=(const AggressivePlayerStrategy& other);
        ~AggressivePlayerStrategy();
        void issueOrder() override;
        vector<Territory*> toAttack() override;
        vector<Territory*> toDefend() override;

        PlayerStrategy* clone() const override;
        friend ostream& operator<<(ostream& out, const AggressivePlayerStrategy& aps);
};

class BenevolentPlayerStrategy : public PlayerStrategy {
    public:
        BenevolentPlayerStrategy(Player* player);
        BenevolentPlayerStrategy(const BenevolentPlayerStrategy& other);
        BenevolentPlayerStrategy& operator=(const BenevolentPlayerStrategy& other);
        ~BenevolentPlayerStrategy();
        void issueOrder() override;
        vector<Territory*> toAttack() override;
        vector<Territory*> toDefend() override;

        PlayerStrategy* clone() const override;
        friend ostream& operator<<(ostream& out, const BenevolentPlayerStrategy& bps);
};

class NeutralPlayerStrategy : public PlayerStrategy {
    public:
        NeutralPlayerStrategy(Player* player);
        NeutralPlayerStrategy(const NeutralPlayerStrategy& other);
        NeutralPlayerStrategy& operator=(const NeutralPlayerStrategy& other);
        ~NeutralPlayerStrategy();
        void issueOrder() override;
        vector<Territory*> toAttack() override;
        vector<Territory*> toDefend() override;

        PlayerStrategy* clone() const override;
        friend ostream& operator<<(ostream& out, const NeutralPlayerStrategy& nps);
};
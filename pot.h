#ifndef POT_H
#define POT_H

#include "common.h"

class Pot
{
    int totalPot;
    int bet;
    std::vector<std::shared_ptr<Player>> players;
public:
    Pot(int pot = 0, int bet = 0) : totalPot(pot), bet(bet) {}

    void addPlayer(std::shared_ptr<Player>);
};

#endif

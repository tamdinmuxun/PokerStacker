#ifndef POT_H
#define POT_H

#include "common.h"

class Pot
{
    int total_pot;
    int bet;
    std::vector<std::shared_ptr<Player>> players;
public:
    Pot(int pot = 0) : total_pot(pot) {}

    void addPlayer(std::shared_ptr<Player>);
    void addPlayers(const std::vector<std::shared_ptr<Player>> &);
    void add(int bet);

    int getPot();
    std::vector<std::shared_ptr<Player>> getPlayers();
};

#endif

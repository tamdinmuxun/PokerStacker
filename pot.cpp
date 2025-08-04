#include "pot.h"

void Pot::addPlayer(std::shared_ptr<Player> player)
{
    players.push_back(player);
}

void Pot::addPlayers(const std::vector<std::shared_ptr<Player>> &ps)
{
    players = ps;
}

void Pot::add(int bet)
{
    total_pot += bet;
}

int Pot::getPot()
{
    return total_pot;
}

std::vector<std::shared_ptr<Player>> Pot::getPlayers()
{
    return players;
}

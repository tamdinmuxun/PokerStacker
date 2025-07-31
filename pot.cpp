#include "pot.h"

void Pot::addPlayer(std::shared_ptr<Player> player)
{
    players.push_back(player);
}

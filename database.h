#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"
#include "player.h"
#include "room.h"
#include <pqxx/pqxx>

class Database
{
    std::shared_ptr<pqxx::connection> conn;
public:
    Database(const std::string &s);

    void createPlayer(int64_t userId, const std::string &name);

    std::shared_ptr<Player> getPlayer(int64_t userId);

    void createRoom(const std::string &id, int64_t ownerId);

    void updateRoomChips(const std::string &id, int chips);

    std::vector<std::shared_ptr<Room>> getRooms(std::shared_ptr<Player>);
};

#endif

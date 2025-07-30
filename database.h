#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"
#include "player.h"
#include "room.h"
#include <pqxx/pqxx>

class Database
{
    shared_ptr<pqxx::connection> conn;
public:
    Database(const string &s);

    void createPlayer(int64_t userId, const string &name);

    shared_ptr<Player> getPlayer(int64_t userId);

    void updatePlayerChips(int64_t userId, int chips);

    void createRoom(const string &id, int64_t ownerId);

    void updateRoomChips(const string &id, int chips);

    vector<shared_ptr<Room>> getRooms(shared_ptr<Player>);
};

#endif

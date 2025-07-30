#include "database.h"

Database::Database(const string &s)
{
    try {
        conn = std::make_shared<pqxx::connection>(s);
        std::cout << "Connected to: " << conn->dbname() << std::endl;
    } catch (const exception& e) {
        std::cerr << "DB error: " << e.what() << std::endl;
        // throw;
    }
}

void Database::createPlayer(int64_t userId, const string &name)
{
    pqxx::work txn(*conn);
    txn.exec(
        "INSERT INTO players (user_id, name) VALUES ($1, $2) "
        "ON CONFLICT (user_id) DO NOTHING",
        pqxx::params(userId, name)).no_rows();
    txn.commit();

}

shared_ptr<Player> Database::getPlayer(int64_t userId)
{
    pqxx::work txn(*conn);

    pqxx::result res = txn.exec("SELECT name, chips, wins FROM players WHERE user_id=$1", pqxx::params(userId));

    if (res.empty()) {
        return nullptr;
    }

    auto player = make_shared<Player>(nullptr, userId, res[0]["name"].as<string>(), res[0]["chips"].as<int>());
    player->setWins(res[0]["wins"].as<int>());
    return player;
}

void Database::updatePlayerChips(int64_t userId, int chips)
{
    pqxx::work txn(*conn);

    txn.exec("UPDATE players SET chips=$1 WHERE user_id=$2", pqxx::params(chips, userId)).no_rows();
    txn.commit();
}

void Database::createRoom(const string &id, int64_t ownerId)
{
    pqxx::work txn(*conn);

    txn.exec("INSERT INTO rooms (id, owner_id) VALUES ($1, $2)"
                     "ON CONFLICT (id) DO NOTHING",
                     pqxx::params(id, ownerId)).no_rows();
    txn.commit();
}

void Database::updateRoomChips(const string &id, int chips)
{
    pqxx::work txn(*conn);

    txn.exec("UPDATE rooms SET initial_chips=$1 WHERE id=$2", pqxx::params(chips, id)).no_rows();

    txn.commit();
}

vector<shared_ptr<Room>> Database::getRooms(shared_ptr<Player> player)
{
    pqxx::work txn(*conn);
    vector<shared_ptr<Room>> playerRooms;

    pqxx::result res = txn.exec("SELECT id, initial_chips FROM rooms WHERE owner_id=$1", pqxx::params(player->getId()));
    printf("Number of rooms: %d\n", res.size());
    if (!res.empty()) 
        for (pqxx::row line : res) {
            printf("id of room: %s\n", line["id"].as<string>().c_str());
            playerRooms.push_back(make_shared<Room>(player, line["id"].as<string>(), line["initial_chips"].as<int>()));
        }
    return playerRooms;
}

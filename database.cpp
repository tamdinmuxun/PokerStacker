#include "database.h"

Database::Database(const std::string &s)
{
    try {
        conn = std::make_shared<pqxx::connection>(s);
        std::cout << "Connected to: " << conn->dbname() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "DB error: " << e.what() << std::endl;
        // throw;
    }
}

void Database::createPlayer(int64_t user_id, const std::string &name)
{
    pqxx::work txn(*conn);
    txn.exec(
        "INSERT INTO players (user_id, name) VALUES ($1, $2) "
        "ON CONFLICT (user_id) DO NOTHING",
        pqxx::params(user_id, name)).no_rows();
    txn.commit();

}

std::shared_ptr<Player> Database::getPlayer(int64_t user_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    pqxx::work txn(*conn);

    pqxx::result res = txn.exec("SELECT name, wins FROM players WHERE user_id=$1", pqxx::params(user_id));

    if (res.empty()) {
        return std::shared_ptr<Player>(nullptr);
    }

    auto player = std::make_shared<Player>(nullptr, user_id, res[0]["name"].as<std::string>());
    player->setWins(res[0]["wins"].as<int>());
    return player;
}

void Database::createRoom(const std::string &id, int64_t owner_id)
{
    pqxx::work txn(*conn);

    txn.exec("INSERT INTO rooms (id, owner_id) VALUES ($1, $2)"
             "ON CONFLICT (id) DO NOTHING",
             pqxx::params(id, owner_id)).no_rows();
    txn.commit();
}

std::pair<int64_t, std::shared_ptr<Room>> Database::getRoom(const std::string &id)
{
    pqxx::work txn(*conn);

    pqxx::result res = txn.exec("SELECT owner_id, initial_chips FROM rooms WHERE id=$1", pqxx::params(id));

    if (res.empty()) {
        return {0, std::shared_ptr<Room>(nullptr)};
    }

    return {res[0]["owner_id"].as<int64_t>(), std::make_shared<Room>(nullptr, id, res[0]["initial_chips"].as<int>())};
}

void Database::updateRoomOwner(const std::string &id, int64_t owner_id)
{
    pqxx::work txn(*conn);

    txn.exec("UPDATE rooms SET owner_id=$1 WHERE id=$2", pqxx::params(owner_id, id)).no_rows();

    txn.commit();
}

void Database::updateRoomChips(const std::string &id, int chips)
{
    pqxx::work txn(*conn);

    txn.exec("UPDATE rooms SET initial_chips=$1 WHERE id=$2", pqxx::params(chips, id)).no_rows();

    txn.commit();
}

std::vector<std::shared_ptr<Room>> Database::getRooms(std::shared_ptr<Player> player)
{
    pqxx::work txn(*conn);
    std::vector<std::shared_ptr<Room>> playerRooms;
    pqxx::result res = txn.exec("SELECT id, initial_chips FROM rooms WHERE owner_id=$1", pqxx::params(player->getId()));
    printf("Number of rooms: %d\n", res.size());
    if (!res.empty()) 
        for (pqxx::row line : res) {
            printf("id of room: %s\n", line["id"].as<std::string>().c_str());
            playerRooms.push_back(std::make_shared<Room>(player, line["id"].as<std::string>(), line["initial_chips"].as<int>()));
        }
    return playerRooms;
}

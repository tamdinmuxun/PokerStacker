#ifndef MY_BOT
#define MY_BOT

#include "room.h"
#include "database.h"
#include <format>
#include <unordered_map>

class MyBot
{
    Bot bot;
    static Database db;

    static std::unordered_map<int64_t, std::shared_ptr<Player>> players;
    static std::unordered_map<std::string, std::shared_ptr<Room>> rooms;
public:
    MyBot(std::string token) : bot(token) {
        setCommands();
        setBotCommands();
    }

    static std::pair<std::shared_ptr<Player>, std::shared_ptr<Room>> getPlayerRoom(Bot &bot, int64_t userId);

    static void regPlayer(Bot &bot, int64_t userId);

    static void handleStart(Bot &bot, int64_t userId);

    static void handleJoin(Bot &bot, int64_t userId);

    static void handleCreate(Bot &bot, int64_t userId);

    static void handleLeave(Bot &bot, int64_t userId);

    static void handleGamble(Bot &bot, int64_t userId);

    static void handleCall(Bot &bot, int64_t userId);

    static void handleRaise(Bot &bot, int64_t userId);

    static void handleFold(Bot &bot, int64_t userId);

    static void handleCheck(Bot &bot, int64_t userId)
    {
        handleCall(bot, userId);
    }

    static void handleBet(Bot &bot, int64_t userId)
    {
        handleRaise(bot, userId);
    }

    static void handleStats(Bot &bot, int64_t userId);

    static void handleName(Bot &bot, int64_t userId, const std::string &name);

    static void handleId(Bot &bot, int64_t userId, const std::string &id);

    static void handleChips(Bot &bot, int64_t userId, const std::string &text);

    static void handleRaising(Bot &bot, int64_t userId, const std::string &text);

    static void handleWinner(Bot &bot, int64_t userId, const std::string &text);

    static void handleMessage(Bot &bot, Message::Ptr message);


    void setCommands();

    void setBotCommands();

    void skipPendingUpdates(Bot& bot);

    void run();
};

#endif

#ifndef MULTI_PLAYER_ROOM
#define MULTI_PLAYER_ROOM

#include "player.h"
#include <vector>

enum class GameState {
    PREFLOP,
    FLOP,
    TURN,
    RIVER,
    SHOWDOWN,
    FINAL
};

class Room
{
    shared_ptr<Player> owner;
    string id;
    int initialChips;
    bool gameStarted{false};
    int64_t totalPot{};
    vector<shared_ptr<Player>> order;
    int diller{-1};
    int currentBet{};
    int sb{-1}, bb{-1};
    int curPlayer{};
    int last{};
    GameState state{GameState::PREFLOP};
public:
    Room(shared_ptr<Player> owner = nullptr, 
         string _id = "",
         int initialChips = 0,
         vector<shared_ptr<Player>> order = vector<shared_ptr<Player>>()) :
    owner(owner),
    initialChips(initialChips)
    {
        if (_id == "") {
            id = generateRoomId();
        } else {
            id = _id;
        }
    }

    string getId() const noexcept
    {
        return id;
    }

    void setChips(int chips) noexcept
    {
        if (chips < 0) {
            fprintf(stderr, "Error! Initial number of chips can't be negative!\n");
            return;
        }
        initialChips = chips;
    }

    int getInitialChips() const noexcept
    {
        return initialChips;
    }

    int getCurrentBet() const noexcept
    {
        return currentBet;
    }

    void setBet(int bet)
    {
        currentBet = bet;
    }

    void addPlayer(shared_ptr<Player> player);

    void removePlayer(shared_ptr<Player> player);

    void nextPlayer();

    void updateLast();

    void startGame();

    void preflop();

    void startRound();

    void betting();

    void endRound();

    void endGame(int winner = -1);

    void stats() const noexcept;

    bool empty() const noexcept
    {
        return order.size() == 0;
    }

    int size() const noexcept
    {
        return order.size();
    }

    string to_string();

    ~Room()
    {
        for (auto player : order) {
            player->sendMessage("Комната закрыта!");
        }
    }
};
#endif

#ifndef MULTI_PLAYER_ROOM
#define MULTI_PLAYER_ROOM

#include "player.h"
#include "pot.h"
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
    std::shared_ptr<Player> owner;
    std::string id;
    int initial_chips;
    int64_t total_pot{};
    std::vector<std::shared_ptr<Player>> order{};
    int diller{-1};
    int current_bet{};
    int sb{-1}, bb{-1};
    int cur_player{};
    int last{};
    int cur_pot{};
    GameState state{GameState::PREFLOP};
    std::vector<std::shared_ptr<Pot>> pots{};
public:
    Room(std::shared_ptr<Player> owner = nullptr, 
         std::string _id = "",
         int initial_chips = 0) :
    owner(owner),
    initial_chips(initial_chips)
    {
        if (_id == "") {
            id = generateRoomId();
        } else {
            id = _id;
        }
        pots.push_back(std::make_shared<Pot>());
    }

    std::string getId() const noexcept
    {
        return id;
    }

    void setOwner(std::shared_ptr<Player> player)
    {
        owner = player;
    }

    void setChips(int chips) noexcept
    {
        if (chips < 0) {
            fprintf(stderr, "Error! Initial number of chips can't be negative!\n");
            return;
        }
        initial_chips = chips;
    }

    int getInitialChips() const noexcept
    {
        return initial_chips;
    }

    int getCurrentBet() const noexcept
    {
        return current_bet;
    }

    void setBet(int bet)
    {
        current_bet = bet;
    }

    void setStates(PlayerState state) {
        for (auto player : order) {
            player->setState(state);
        }
    }
    bool addPlayer(std::shared_ptr<Player> player);

    void removePlayer(std::shared_ptr<Player> player);

    int activePlayers();

    int playingPlayers();

    std::vector<std::shared_ptr<Player>> getActivePlayers();

    bool nextPlayer();

    void updateLast();

    void startGame();

    void preflop();

    void startRound();

    void betting();

    void endRound();

    void endGame();

    bool distributePot(const std::vector<int> &winners);

    void results();

    void sendMessageToAll(const std::string &) const noexcept;

    void sendMessageToAllExcept(const std::string&, std::shared_ptr<Player>) const noexcept;

    void stats() const noexcept;

    bool empty() const noexcept
    {
        return order.size() == 0;
    }

    int size() const noexcept
    {
        return order.size();
    }

    std::string to_string();

    ~Room()
    {
        for (auto player : order) {
            player->sendMessage("Комната закрыта!");
        }
    }
};
#endif

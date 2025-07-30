#ifndef PLAYER
#define PLAYER

#include "common.h"
#include <memory>

enum class PlayerState {
    IDLE,
    NAME,
    CHIPS,
    ID,
    WAITING,
    GAMBLING,
    RAISING,
    WINNER,
    RAISED,
    CALLED,
    CHECKED,
    BET,
    FOLDED
};

class Player
{
    Bot *bot;
    int64_t id;
    PlayerState state;
    string name;
    int chips;
    int currentBet;
    bool fold{};
    bool allIn{};
    weak_ptr<Room> room{shared_ptr<Room>(nullptr)};
    int wins{};
public:
    Player(Bot *bot = nullptr,
           int64_t id = 0,
           const string &name = "",
           int chips = 1000,
           PlayerState state = PlayerState::IDLE,
           int currentBet = 0) :
    bot(bot),
    id(id),
    name(name),
    chips(chips),
    currentBet(currentBet),
    state(state)
    {}

    void sendMessage(string s)
    {
        try {
            if (bot) {
                bot->getApi().sendMessage(id, s);
            }
        } catch (...) {
            cerr << "Error sending message to " << id << endl;
        }
    }

    void setBot(Bot *b)
    {
        bot = b;
    }

    PlayerState getState() const noexcept
    {
        return state;
    }

    void setWins(int w) noexcept
    {
        if (wins < 0) {
            fprintf(stderr, "Error! Negative number of wins\n");
            return;
        }
        wins = w;
    }

    int getWins() const noexcept
    {
        return wins;
    }

    void setState(PlayerState s)
    {
        state = s;
    }

    void setName(const string &s) {
        name = s;
    }

    string getName() const noexcept
    {
        return name;
    }

    int64_t getId() const noexcept
    {
        return id;
    }

    auto getRoom() const noexcept
    {
        return room.lock();
    }

    void setRoom(shared_ptr<Room> r)
    {
        room = r;
    }

    int getCurrentBet() const noexcept
    {
        return currentBet;
    }

    int getChips() const noexcept
    {
        return chips;
    }

    void setChips(int c) noexcept
    {
        if (c < 0) {
            fprintf(stderr, "Error! Player can't have negative number of chips\n");
            return;
        }
        chips = c;
    }

    void addChips(int c) noexcept
    {
        chips += c;
    }

    bool isFolded() const noexcept
    {
        return fold;
    }

    bool isAllIn() const noexcept
    {
        return allIn;
    }

    void setFold(bool f)
    {
        fold = f;
        if (f) 
            sendMessage("Вы сбросили карты.");
    }

    void setAllIn(bool f)
    {
        allIn = f;
    }

    string getStatus() const noexcept;

    void small();

    void big();

    void notifyPlayer(int bet);

    void makeBet(int bet = 0);

    int commit();

    string to_string();
};
#endif

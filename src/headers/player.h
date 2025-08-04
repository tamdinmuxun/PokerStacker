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
    FOLDED,
    ALL_IN,
    REGAMBLING
};

class Player
{
    Bot *bot;
    int64_t id;
    PlayerState state;
    std::string name;
    int chips;
    int current_bet;
    bool fold{};
    bool all_in{};
    std::weak_ptr<Room> room{std::shared_ptr<Room>(nullptr)};
    int wins{};
public:
    Player(Bot *bot = nullptr,
           int64_t id = 0,
           const std::string &name = "",
           int chips = 1000,
           PlayerState state = PlayerState::IDLE,
           int current_bet = 0) :
    bot(bot),
    id(id),
    name(name),
    chips(chips),
    current_bet(current_bet),
    state(state)
    {}

    void sendMessage(std::string s) const noexcept
    {
        try {
            if (bot) {
                bot->getApi().sendMessage(id, s);
            }
        } catch (std::exception &e) {
            printf("%s\n", e.what());
        }
    }

    auto getBot() const noexcept
    {
        return bot;
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

    void addWins() noexcept
    {
        ++wins;
    }

    int getWins() const noexcept
    {
        return wins;
    }

    void setState(PlayerState s)
    {
        state = s;
    }

    void setName(const std::string &s) {
        name = s;
    }

    std::string getName() const noexcept
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

    void setRoom(std::shared_ptr<Room> r)
    {
        room = std::weak_ptr<Room>(r);
    }

    int getCurrentBet() const noexcept
    {
        return current_bet;
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
        return all_in;
    }

    void setFold(bool f)
    {
        fold = f;
        if (f) 
            sendMessage("Вы сбросили карты.");
    }

    void setAllIn(bool f)
    {
        all_in = f;
    }

    std::string getStatus() const noexcept;

    void small();

    void big();

    void notifyPlayer(int bet);

    void makeBet(int bet = 0);

    int commit();

    std::string to_string();
};
#endif

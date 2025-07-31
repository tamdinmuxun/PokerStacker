#include "room.h"

std::string Room::to_string()
{
    return std::format("Идентификатор комнаты: {}\nНачальный стек: {}.", id, initial_chips);
}

void Room::addPlayer(std::shared_ptr<Player> player)
{
    order.push_back(player);

    for (auto p : order) {
        p->sendMessage("Игрок " + player->getName() + " присоединился к комнате!" +
            "Текущеее количество игроков: " + std::to_string(order.size()));
    }
}

void Room::removePlayer(std::shared_ptr<Player> player)
{

    for (auto p = order.begin(); p != order.end(); ++p) {
        if ((*p)->getId() == player->getId()) {
            order.erase(p);
            break;
        }
    }
    player->setRoom(nullptr);

    for (auto p : order) {
        p->sendMessage("Игрок " + player->getName() + " вышел из комнаты(." +
        "Текущеее количество игроков: " + std::to_string(order.size()));
    }

    if (player->getId() == owner->getId() && order.size()) {
        owner = order[0];
        for (auto p : order) {
            p->sendMessage("А он был создателем. Теперь права админа переходят игроку " + 
                order[0]->getName() + ".");

        }
    }
}

void Room::nextPlayer()
{
    do {
        cur_player = (cur_player + 1) % order.size(); 
    } while (cur_player != last && (order[cur_player]->isFolded() || order[cur_player]->isAllIn()));
    
    if (cur_player == last && (order[cur_player]->isFolded() || order[cur_player]->isAllIn())) {
        endRound();
    }
}

void Room::updateLast()
{
    last = (cur_player - 1 + order.size()) % order.size();
}

void Room::startGame()
{
    if (order.size() < 2) {
        throw NotEnough();
    }
    for (auto player : order) {
        player->setChips(initial_chips);
    }
    diller = diller == -1 ? rnd(rng) % order.size() : (diller + 1) % order.size();
    order[diller]->sendMessage("В этой игре вы диллер.");

    for (auto player : order) {
        player->setState(PlayerState::GAMBLING);
        player->sendMessage("Игра началась! Ваш начальный баланс: " +
            std::to_string(initial_chips) + ". Диллер: " + order[diller]->getName());
    }
    
    preflop();
}

void Room::preflop()
{
    state = GameState::PREFLOP;
    sb = (diller + 1) % order.size();
    bb = (diller + 2) % order.size();
    // notify
    for (auto player : order) {
        player->sendMessage("Малый блайнд 5 денег: " + order[sb]->getName() +
            + ".\nБольшой блайнд 10 денег: " + order[bb]->getName() + ".");
    }

    order[sb]->small();
    order[bb]->big();
    current_bet = BIG_BLIND;
    cur_player = (bb + 1) % order.size();
    last = bb;

    order[cur_player]->notifyPlayer(current_bet);
}

void Room::startRound()
{
    current_bet = 0;
    cur_player = (diller + 1) % order.size();
    updateLast();

    order[cur_player]->notifyPlayer(current_bet);
}

void Room::betting()
{
    int folded = 0;
    int winner = -1;
    for (int i = 0; i < order.size(); ++i) {
        if (order[i]->isFolded()) 
            ++folded;
        else 
            winner = i; 
    }
    if (folded == order.size() - 1) {
        endGame(winner);
    } else if (cur_player == last) {
        endRound();
    } else {
        nextPlayer();
        order[cur_player]->notifyPlayer(current_bet);
    }
}

void Room::endRound()
{
    for (auto player : order) {
        total_pot += player->commit();
    }
    for (auto player : order) {
        player->sendMessage("Раунд окончен. Общий банк: " + 
            std::to_string(total_pot) + " фишек.");
    }
    if (state == GameState::FINAL) return;

    state = GameState(static_cast<int>(state) + 1);
    switch (state) {
    case GameState:: SHOWDOWN:
        endGame();
        break;
    default:
        startRound();
    }
}

void Room::endGame(int winner)
{
    if (winner == -1) {
        int cnt = 1;
        std::string message = "Кто победил?\n";
        for (auto player : order) {
            message += std::to_string(cnt++) + ". " + player->getName() + "\n";
        }
        owner->sendMessage(message);
        owner->setState(PlayerState::WINNER);
    } else {
        state = GameState::FINAL;
        endRound();
        order[winner]->addChips(total_pot);

        for (auto player : order) {
            player->sendMessage("Победитель: " + order[winner]->getName() + "!");
        }
    }
}

void Room::stats() const noexcept
{
    std::string message = "Начальный стек: " + std::to_string(initial_chips) + " фишек\n";
    message += "Игроки за столом:\n";

    int index = 1;
    for (const auto& player : order) {
        message += std::to_string(index++) + ". " + player->getName() + "\nВсего:" + std::to_string(player->getChips()) + " фишек\n" +
        "Текущая ставка: " + std::to_string(player->getCurrentBet()) + "\nСтатус: " + player->getStatus() + "\n";
    }
    
    for (const auto &player : order) {
        player->sendMessage(message);
    }
}

#include "room.h"

std::string to_string(GameState state)
{
    switch(state) {
    case GameState::PREFLOP:    return "Префлоп";
    case GameState::FLOP:       return "Флоп. Выложите 3 карты на стол.";
    case GameState::TURN:       return "Тёрн. Выложите 4-ую карту на стол.";
    case GameState::RIVER:      return "Ривер. Выложите 5-ую карту на стол.";
    case GameState::SHOWDOWN:   return "Шоудаун. Вскрываемся.";
    case GameState::FINAL:      return "Финал.";
    }
}

std::string Room::to_string()
{
    return std::format("Идентификатор комнаты: {}\nНачальный стек: {}.", id, initial_chips);
}

void Room::sendMessageToAll(const std::string &message) const noexcept
{
    for (auto p : order) {
        p->sendMessage(message);
    }
}

void Room::sendMessageToAllExcept(const std::string &message, std::shared_ptr<Player> player) const noexcept
{
    for (auto p : order) {
        if (p->getId() != player->getId()) {
            p->sendMessage(message);
        }
    }
}

bool Room::addPlayer(std::shared_ptr<Player> player)
{
    for (auto p : order) {
        if (p->getId() == player->getId()) {
            p->sendMessage("Вы уже находитесь в данной комнате!");
            return false;
        }
    }
    player->setChips(initial_chips);
    order.push_back(player);
    sendMessageToAllExcept(std::format("Игрок {} присоединился к комнате!\nТекущеее количество игроков: {}",
        player->getName(), order.size()), player);
    return true;
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

    sendMessageToAll(std::format("Игрок {} вышел из комнаты(.\nТекущеее количество игроков: {}",
        player->getName(), order.size()));

    if (player->getId() == owner->getId() && order.size()) {
        owner = order[0];
        sendMessageToAll(std::format("А он был создателем. Теперь права админа переходят игроку {}.", order[0]->getName()));
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

    sendMessageToAll(std::format("Игра началась! Ваш начальный баланс: {}. Диллер: {}.", initial_chips, order[diller]->getName()));

    for (auto player : order) {
        player->setState(PlayerState::GAMBLING);
    }
    
    preflop();
}

void Room::preflop()
{
    sendMessageToAll("Ставки сразу после раздачи карт.");
    state = GameState::PREFLOP;
    sb = (diller + 1) % order.size();
    bb = (diller + 2) % order.size();

    sendMessageToAll(std::format("Малый блайнд 5 денег: {}.\nnБольшой блайнд 10 денег: {}.", order[sb]->getName(), order[bb]->getName()));

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
    sendMessageToAll(std::format("Раунд окончен. Общий банк: {} фишек.", total_pot));

    if (state == GameState::FINAL) return;

    state = GameState(static_cast<int>(state) + 1);
    sendMessageToAll(::to_string(state));
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
            message += std::format("{}. {}\n", cnt++, player->getName());
        }
        owner->sendMessage(message);
        owner->setState(PlayerState::WINNER);
    } else {
        state = GameState::FINAL;
        endRound();
        order[winner]->addChips(total_pot);

        sendMessageToAll(std::format("Победитель: {}!", order[winner]->getName()));
    }
}

void Room::stats() const noexcept
{
    std::string message = std::format("Начальный стек: {} фишек\n", initial_chips);
    message += "Игроки за столом:\n";

    int index = 1;
    for (const auto& player : order) {
        message += std::format("{}. {}\nВсего: {} фишек\nТекущая ставка: {}\nСтатус: {}.\n", index++, player->getName(), player->getChips(), player->getCurrentBet(), player->getStatus());
    }
    sendMessageToAll(message);
}

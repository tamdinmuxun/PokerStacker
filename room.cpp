#include "room.h"

std::string to_string(GameState state)
{
    switch(state) {
    case GameState::PREFLOP:    return "Префлоп. Ставки сразу после раздачи карт.";
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

bool Room::nextPlayer()
{
    do {
        cur_player = (cur_player + 1) % order.size(); 
    } while (cur_player != last && (order[cur_player]->isFolded() || order[cur_player]->isAllIn() || !order[cur_player]->getChips()));
    
    if (cur_player == last && (order[cur_player]->isFolded() || order[cur_player]->isAllIn())) {
        endRound();
        return false;
    }
    return true;
}

void Room::updateLast()
{
    last = (cur_player - 1 + order.size()) % order.size();
}

int Room::activePlayers()
{
    int ans = 0;
    for (auto player : order) {
        if (!player->isFolded() && (player->getChips() > 0 || (player->getChips() == 0 && player->isAllIn()))) {
            ++ans;
        }
    }
    return ans;
}

int Room::playingPlayers()
{
    int ans = 0;
    for (auto player : order) {
        if (!player->isFolded() && !player->isAllIn()) {
            ++ans;
        }
    }
    return ans;
}

std::vector<std::shared_ptr<Player>> Room::getActivePlayers()
{
    std::vector<std::shared_ptr<Player>> active_players;
    for (auto player : order) {
        if (!player->isFolded()) {
            active_players.push_back(player);
        }
    }
    std::sort(active_players.begin(), active_players.end(),
        [] (auto a, auto b)
        {
            return a->getCurrentBet() < b->getCurrentBet();
        }
    );
    return active_players;
}

void Room::startGame()
{
    if (order.size() < 2) {
        throw NotEnough();
    }
    for (auto player : order) {
        if (player->getState() != PlayerState::REGAMBLING) player->setChips(initial_chips);
        player->sendMessage(std::format("Ваш баланс: {}", player->getChips()));
    }
    diller = diller == -1 ? rnd(rng) % order.size() : (diller + 1) % order.size();
    order[diller]->sendMessage("В этой игре вы диллер.");

    sendMessageToAll(std::format("Игра началась! Диллер: {}.", order[diller]->getName()));

    for (auto player : order) {
        player->setState(PlayerState::GAMBLING);
    }
    
    preflop();
}

void Room::preflop()
{
    state = GameState::PREFLOP;
    sendMessageToAll(::to_string(state));
    sb = (diller + 1) % order.size();
    bb = (diller + 2) % order.size();

    sendMessageToAll(std::format("Малый блайнд 5 денег: {}.\nБольшой блайнд 10 денег: {}.", order[sb]->getName(), order[bb]->getName()));

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
    printf("%s\n", __PRETTY_FUNCTION__);
    printf("folded %d\n", order[cur_player]->isFolded());
    if (order[cur_player]->isFolded()) {
        printf("folded %d\n", order[cur_player]->isFolded());
    
        int bet = order[cur_player]->commit();
        total_pot += bet;
        pots.back()->add(bet);
        if (activePlayers() == 1) {
            printf("success\n");
            fflush(stdout);
            endRound();
            return;
        }
    } else if (order[cur_player]->isAllIn()) {
        printf("All in %d\n", order[cur_player]->isAllIn());
    }
    if (cur_player == last) {
        endRound();
        return;
    } else {
        if (nextPlayer())
            order[cur_player]->notifyPlayer(current_bet);
    }
}

void Room::endRound()
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto active_players = getActivePlayers();
    int num_active = activePlayers();
    printf("num active %d\nactive_players.size() %zu\n", num_active, active_players.size());
    int prev = 0;
    int early_stop = (num_active == 1);
    for (int i = 0; i < active_players.size(); ++i) {
        auto player = active_players[i];
        if (player->getCurrentBet() > prev) {
            pots.back()->add((player->getCurrentBet() - prev) * num_active);
            pots.back()->addPlayers(active_players);
            --num_active;
            prev = player->getCurrentBet();
            if (player->isAllIn()) {
                active_players.erase(find(active_players.begin(), active_players.end(), player));
                if (!early_stop && playingPlayers() > 1) {
                    pots.push_back(std::make_shared<Pot>());
                }
            }
        }
        total_pot += player->commit();

        printf("%s : %d\n", player->getName().c_str(), player->getChips());
    }

    sendMessageToAll(std::format("Раунд окончен. Общий банк: {} фишек.", total_pot));
    if (early_stop) {
        distributePot({0});
        return;
    }
    if (playingPlayers() == 1) {
        sendMessageToAll("Выложите оставшиеся карты.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        results();
        return;
    }
    if (state == GameState::FINAL) return;

    state = GameState(static_cast<int>(state) + 1);
    sendMessageToAll(::to_string(state));
    switch (state) {
    case GameState:: SHOWDOWN:
        results();
        break;
    default:
        startRound();
    }
    
}

void Room::results()
{
    std::string message = "Назовите победителей каждого банка.\n";
    message += "Банки:\n";
    int i = 1;
    for (auto pot : pots) {
        message += std::format("Банк № {}. Сумма: {}. Список игроков, участвующих в розыгрыше банка:\n", i++, pot->getPot());
        int j = 1;
        for (auto player : pot->getPlayers()) {
            message += std::format("{}. {}\n", j++, player->getName());
        }
    }
    message += "Отправьте номера победителей каждого банка в том же порядке, что и в данном сообщении. На один банк одно сообщение.";
    owner->sendMessage(message);
    owner->setState(PlayerState::WINNER);
}

bool Room::distributePot(const std::vector<int> &winners)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    if (cur_pot >= pots.size()) {
        cur_pot = 0;
        endGame();
    }
    auto pot = pots[cur_pot];
    int n = winners.size();
    int prize = pot->getPot() / n;
    auto players = pot->getPlayers();
    std::string message = std::format("Банк № {}. Победители:\n", cur_pot + 1);
    for (int i = 0; i < winners.size(); ++i) {
        int sum = prize + (i == 0 ? pot->getPot() % n : 0);
        printf("Prize: %d\n", sum);
        auto player = players[winners[i]];
        player->addChips(sum);
        player->addWins();
        message += std::format("Игрок {} выиграл {} фишек!", player->getName(), sum);
    }
    sendMessageToAll(message);
    ++cur_pot;
    printf("cur_pot: %d\npot.size: %d\n", cur_pot, pots.size());
    bool stop = (cur_pot == pots.size());
    if (stop) {
        endGame();
    }
    return stop;
}

void Room::endGame()
{
    sendMessageToAll("Игра окончена. Если хотите продолжить игру, отправьте команду /gamble.");
    total_pot = 0;
    state = GameState::FINAL;
    pots.clear();
    pots.push_back(std::make_shared<Pot>());
    cur_pot = 0;
    for (auto player : order) {
        player->setFold(false);
        player->setAllIn(false);
        // TODO UPDATE PLAYER
    }
    setStates(PlayerState::REGAMBLING);
}

void Room::stats() const noexcept
{
    std::string message = std::format("Начальный стек: {} фишек\n", initial_chips);
    message += "Игроки за столом:\n";

    int index = 1;
    for (const auto& player : order) {
        message += std::format("{}. {}\n", index++, player->to_string());
    }
    sendMessageToAll(message);
}

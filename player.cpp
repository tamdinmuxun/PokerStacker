#include "player.h"

std::string Player::to_string()
{
    return std::format("Игрок: {}\nБаланс: {}\nТекущая ставка: {}\nКоличество побед: {}\nСтатус: {}.",
           name, chips, current_bet, wins, getStatus());
}

std::string Player::getStatus() const noexcept
{
    switch (state) {
    case PlayerState::WAITING:
        return "Жду своего хода.";
    case PlayerState::CHECKED:
        return "Ничего не поставил (сделал чек).";
    case PlayerState::CALLED:
        return "Уравнял ставку.";
    case PlayerState::RAISED:
        return "Повысил ставку.";
    case PlayerState::BET:
        return "Сделал ставку.";
    case PlayerState::FOLDED:
        return "Сбросил карты.";
    case PlayerState::GAMBLING:
        return "Жеский гэмблинг.";
    default:
        return "Сплю.";
    }
}

void Player::small()
{
    sendMessage("У вас малый блайнд. Обязательная ставка 5 денег.");
    makeBet(SMALL_BLIND);
}

void Player::big()
{
    sendMessage("У вас большой блайнд. Обязательная ставка 10 денег.");
    makeBet(BIG_BLIND);
}

void Player::notifyPlayer(int bet)
{
    if (bet > 0) {
        sendMessage("Ваш ход. Текущая ставка: " + std::to_string(bet) + 
            ". Отправьте одну из команд:\n/call\n/raise\n/fold"); 
    } else {
        sendMessage("Ваш ход. Никто еще ничего не поставил. \
            Отправьте одну из команд:\n/check\n/bet\n/fold"); 
    }
}

void Player::makeBet(int bet)
{
    if (bet >= chips) {
        all_in = true;
    }
    current_bet = all_in ? chips : bet;

    sendMessage((all_in ? "Ва-Банк! " : "") + std::format("Вы поставили: {} фишек. Осталось: {}.",
                current_bet, chips - current_bet));
}

int Player::commit()
{
    int tmp = current_bet;
    fold = false;
    all_in = false;
    current_bet = 0;
    chips -= tmp;
    return tmp;
}

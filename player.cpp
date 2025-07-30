#include "player.h"

string Player::to_string()
{
    return "Игрок: " + name + "\n" + 
           "Баланс: " + std::to_string(chips) + "\n" +
           "Текущая ставка: " + std::to_string(currentBet) + "\n" + 
           "Количество побед: " + std::to_string(wins) + "\n" + 
           "Статус: " + getStatus();
}

string Player::getStatus() const noexcept
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
    currentBet = bet;
    if (currentBet == chips) {
        allIn = true;
    }
    sendMessage("Вы поставили " + std::to_string(bet) + " фишек. Осталось: " +
        std::to_string(chips - bet) + ".");
}

int Player::commit()
{
    int tmp = currentBet;
    fold = false;
    allIn = false;
    currentBet = 0;
    chips -= tmp;
    return tmp;
}

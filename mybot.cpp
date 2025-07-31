#include "mybot.h"

std::unordered_map<int64_t, std::shared_ptr<Player>> MyBot::players;
std::unordered_map<std::string, std::shared_ptr<Room>> MyBot::rooms;
Database MyBot::db(conn_str);

void MyBot::regPlayer(Bot &bot, int64_t userId)
{
    auto player = MyBot::players[userId] = std::make_shared<Player>(&bot, userId);
    player->sendMessage("Введите Ваше имя:");
    player->setState(PlayerState::NAME);
}

std::pair<std::shared_ptr<Player>, std::shared_ptr<Room>> MyBot::getPlayerRoom(Bot &bot, int64_t userId)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    printf("Trying to retrieve player from database\n");
    if (MyBot::players.count(userId)) {
        printf("Player in cache\n");
        auto player = MyBot::players[userId];
        printf("Is nullptr: %d\n", (player==nullptr));
        auto room = player ? player->getRoom() : nullptr;
        return {player, room};
    } else {
        printf("No player in cache\nRetrieving player from database\n");
        try {
            auto player = MyBot::db.getPlayer(userId);
            printf("Retrieved player from database\n");
            if (!player) {
                MyBot::regPlayer(bot, userId);
            }
            MyBot::players[userId] = player;
            return {player, nullptr};
        } catch (std::exception &obj) {
            printf("%s\n", obj.what());
        }
        return {nullptr, nullptr};
    }
}

void MyBot::handleStart(Bot &bot, int64_t userId)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, room] = getPlayerRoom(bot, userId);
    
    if (!player) {
        printf("No player in database\nRegistering\n");
        regPlayer(bot, userId);
    } else {
        printf("Player found\n");

        if (!player->getBot()) {
            player->setBot(&bot);
        }

        std::string message = "С возвращением, " + player->getName() + "!\n" + 
            "Если хотите создать новую комнату, отправьте команду /create.\n\
Если хотите присоединиться к комнате, отправьте команду /join\nСписок ваших комнат:\n";
        int i = 1;
        printf("Retrieving info about player's rooms\n");
        bool no_rooms = true;
        try {
            for (auto room : db.getRooms(player)) {
                no_rooms = false;
                message += std::format("Комната № {}\n", i++) + room->to_string();
            }
        } catch (std::exception &e) {
            printf("%s\n", e.what());
        }
        if (no_rooms) {
            message += "Пусто";
        }
        printf("Retrieved\n");
        player->sendMessage(message);
    }
}
void MyBot::handleJoin(Bot &bot, int64_t userId)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, room] = getPlayerRoom(bot, userId);
    if (player){
        player->setState(PlayerState::ID);
        player->sendMessage("Введите идентификатор комнаты:");
    } else {
        bot.getApi().sendMessage(userId, "Зарегестрируйтесь!");
    }
}

void MyBot::handleCreate(Bot &bot, int64_t userId)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, r] = getPlayerRoom(bot, userId);
    std::shared_ptr<Room> room = std::make_shared<Room>(player);
    MyBot::rooms[room->getId()] = room;
    // TODO CREATE ROOM AT /SAVE COMMAND 
    // db.createRoom(room->getId(), userId);

    player->setRoom(room);
    player->setState(PlayerState::CHIPS);

    player->sendMessage("Введите начальное количество фишек для каждого игрока:");
}

void MyBot::handleLeave(Bot &bot, int64_t userId)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto player = MyBot::players[userId];

    if (player == nullptr) return; // ERROR PASS

    if (player->getState() == PlayerState::WAITING || player->getState() == PlayerState::GAMBLING) {
        auto room = MyBot::players[userId]->getRoom();
        room->removePlayer(MyBot::players[userId]);

        if (room->empty()) {
            MyBot::rooms.erase(room->getId());
        }
    } else {
        player->sendMessage("Вы не состоите ни в одной комнате");
    }
}

void MyBot::handleGamble(Bot &bot, int64_t userId)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto player = MyBot::players[userId];
    try {
        if (player == nullptr) {
            return;  // ERROR PASS
        }
        auto room = player->getRoom();
        printf("%d\n", room->size());
        room->startGame();
    } catch (NotEnough &) {
        player->sendMessage("Ошибка. Не хватает игроков.");
    } catch (...) {
        player->sendMessage("Error! Can't start game.");
    }
}

void MyBot::handleCall(Bot &bot, int64_t userId)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto player = MyBot::players[userId];
    if (player == nullptr) return;  // ERROR PASS
    auto room = player->getRoom();

    if (room == nullptr) return;  // ERROR PASS

    player->makeBet(room->getCurrentBet());

    room->betting();
}

void MyBot::handleRaise(Bot &bot, int64_t userId)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto player = MyBot::players[userId];
    if (player == nullptr) return;  // ERROR PASS
    auto room = player->getRoom();

    if (room == nullptr) return;  // ERROR PASS
    player->sendMessage("Введите ставку от " + std::to_string(room->getCurrentBet() + 1) +
        " до " + std::to_string(player->getChips()) + ".");
    player->setState(PlayerState::RAISING);
}

void MyBot::handleFold(Bot &bot, int64_t userId)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto player = MyBot::players[userId];
    if (player == nullptr) return;  // ERROR PASS
    auto room = player->getRoom();

    if (room == nullptr) return;  // ERROR PASS

    player->setFold(true);

    room->betting();
}

void MyBot::handleStats(Bot &bot, int64_t userId)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    if (MyBot::players[userId] == nullptr) return;  // ERROR PASS
    MyBot::players[userId]->getRoom()->stats();
}


void MyBot::handleName(Bot &bot, int64_t userId, const std::string &name)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    if (MyBot::players[userId] == nullptr) return;  // ERROR PASS
    MyBot::players[userId]->setName(name);
    MyBot::players[userId]->setState(PlayerState::IDLE);

    db.createPlayer(userId, name);

    bot.getApi().sendMessage(userId,
        "Если хотите создать новую комнату, отправьте команду /create.\n\
Если хотите присоединиться к комнате, отправьте команду /join");
}

void MyBot::handleId(Bot &bot, int64_t userId, const std::string &id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    if (MyBot::rooms.count(id)) {
        if (MyBot::players[userId] == nullptr) return;  // ERROR PASS

        MyBot::rooms[id]->addPlayer(MyBot::players[userId]);
        MyBot::players[userId]->setRoom(MyBot::rooms[id]);
        bot.getApi().sendMessage(userId, "Комната найдена! Вы успешно присоединились!");
        MyBot::players[userId]->setState(PlayerState::WAITING);
    } else {
        bot.getApi().sendMessage(userId, "Неверный идентификатор комнаты, либо ее не существует.");
    }
}

void MyBot::handleChips(Bot &bot, int64_t userId, const std::string &text)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    printf("chips\n");
    try {
        if (MyBot::players[userId] == nullptr) return;  // ERROR PASS

        int chips = std::strtol(text.c_str(), nullptr, 10);
        if (MyBot::players[userId] == nullptr) return;  // ERROR PASS
        auto room = MyBot::players[userId]->getRoom();

        if (room == nullptr) return;  // ERROR PASS
        db.updateRoomChips(room->getId(), chips);
        room->setChips(chips);
        bot.getApi().sendMessage(userId, "Комната создана! Идентификатор: " + room->getId());
        room->addPlayer(MyBot::players[userId]);
        MyBot::players[userId]->setState(PlayerState::WAITING);
    } catch (std::exception &e) {
        printf("%s\n", e.what());
        bot.getApi().sendMessage(userId, "Вы ввели не число, попробуйте еще раз.");
    }
}

void MyBot::handleRaising(Bot &bot, int64_t userId, const std::string &text)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    try {
        int bet = std::strtol(text.c_str(), nullptr, 10);
        auto player = MyBot::players[userId];
        if (player == nullptr) return;  // ERROR PASS
        auto room = player->getRoom();

        if (room == nullptr) return;  // ERROR PASS
        if (bet < room->getCurrentBet()) {
            throw NotEnough();
        } else if (bet > player->getChips()) {
            throw TooMuch();
        }
        player->setState(PlayerState::GAMBLING);
        room->setBet(bet);
        player->makeBet(bet);
        room->updateLast();

        room->betting();
    } catch (NotEnough &) {
        bot.getApi().sendMessage(userId, "Вы ввели число меньше текущей ставки.");
    } catch (TooMuch &) {
        bot.getApi().sendMessage(userId, "Вы не можете поставить больше чем у вас есть.");
    } catch (...) {
        bot.getApi().sendMessage(userId, "Вы ввели не число, попробуйте еще раз.");
    }
}

void MyBot::handleWinner(Bot &bot, int64_t userId, const std::string &text)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    try {
        int i = std::strtol(text.c_str(), nullptr, 10);
        --i;
        auto player = MyBot::players[userId];
        if (player == nullptr) return;  // ERROR PASS

        auto room = MyBot::players[userId]->getRoom();
        
        if (room == nullptr) return;  // ERROR PASS

        room->endGame(i);
        MyBot::players[userId]->setState(PlayerState::IDLE);
    } catch (...) {
        bot.getApi().sendMessage(userId, "Вы ввели не число, попробуйте еще раз.");
    }
}
void MyBot::handleMessage(Bot &bot, Message::Ptr message)
{
    printf("%s\n", __PRETTY_FUNCTION__);

    int64_t userId = message->from->id;
    std::string text = message->text;
    printf("%s\n", text.c_str());
    PlayerState state = MyBot::players.count(userId) ? MyBot::players[userId]->getState() : PlayerState::IDLE;
    printf("message\n");
    switch (state) {
    case PlayerState::NAME:
        handleName(bot, userId, text);
        break;
    case PlayerState::ID:
        handleId(bot, userId, text);
        break;
    case PlayerState::CHIPS:
        handleChips(bot, userId, text);
        break;
    case PlayerState::RAISING:
        handleRaising(bot, userId, text);
        break;
    case PlayerState::WINNER:
        handleWinner(bot, userId, text);
    default:
        break;
    }
}

void MyBot::setCommands()
{
    printf("%s\n", __PRETTY_FUNCTION__);
    bot.getEvents().onCommand("start", [this](Message::Ptr message) { handleStart(bot, message->from->id); });
    bot.getEvents().onCommand("create", [this](Message::Ptr message) { handleCreate(bot, message->from->id); });
    bot.getEvents().onCommand("join", [this](Message::Ptr message) { handleJoin(bot, message->from->id); });
    bot.getEvents().onCommand("leave", [this](Message::Ptr message) { handleLeave(bot, message->from->id); });
    bot.getEvents().onCommand("gamble", [this](Message::Ptr message) { handleGamble(bot, message->from->id); });
    bot.getEvents().onCommand("call", [this](Message::Ptr message) { handleCall(bot, message->from->id); });
    bot.getEvents().onCommand("raise", [this](Message::Ptr message) { handleRaise(bot, message->from->id); });
    bot.getEvents().onCommand("check", [this](Message::Ptr message) { handleCheck(bot, message->from->id); });
    bot.getEvents().onCommand("bet", [this](Message::Ptr message) { handleBet(bot, message->from->id); });
    bot.getEvents().onCommand("fold", [this](Message::Ptr message) { handleFold(bot, message->from->id); });
    bot.getEvents().onCommand("stats", [this](Message::Ptr message) { handleStats(bot, message->from->id); });
    bot.getEvents().onNonCommandMessage([this](Message::Ptr message) { handleMessage(bot, message); });

    bot.getEvents().onCommand("help", [this](Message::Ptr message) {
        std::string helpText = "📜 Доступные команды:\n";
        helpText += "/start - Начать работу\n";
        helpText += "/create - Создать комнату\n";
        helpText += "/join - Присоединиться к игре\n";
        helpText += "/gamble - Начать игру (создатель)\n";
        helpText += "/check - Сделать чек\n";
        helpText += "/call - Уравнять ставку\n";
        helpText += "/bet - Сделать ставку\n";
        helpText += "/raise - Повысить ставку\n";
        helpText += "/fold - Сбросить карты\n";
        helpText += "/stats - Инофрмация о игроках\n";
        helpText += "/leave - Покинуть комнату\n\n";
        helpText += "Во время игры следуйте инструкциям бота!";
        
        bot.getApi().sendMessage(message->chat->id, helpText);

        printf("set\n");
    });
}


void MyBot::setBotCommands()
{
    printf("%s\n", __PRETTY_FUNCTION__);
    try {
        std::vector<BotCommand::Ptr> commands;

        // Создаем команды
        BotCommand::Ptr cmdStart(new BotCommand);
        cmdStart->command = "start";
        cmdStart->description = "Начать работу с ботом";
        commands.push_back(cmdStart);

        BotCommand::Ptr cmdCreate(new BotCommand);
        cmdCreate->command = "create";
        cmdCreate->description = "Создать комнату для игры";
        commands.push_back(cmdCreate);

        BotCommand::Ptr cmdJoin(new BotCommand);
        cmdJoin->command = "join";
        cmdJoin->description = "Присоединиться к комнате";
        commands.push_back(cmdJoin);

        BotCommand::Ptr cmdLeave(new BotCommand);
        cmdLeave->command = "leave";
        cmdLeave->description = "Покинуть комнату";
        commands.push_back(cmdLeave);

        BotCommand::Ptr cmdGamble(new BotCommand);
        cmdGamble->command = "gamble";
        cmdGamble->description = "Начать игру";
        commands.push_back(cmdGamble);

        BotCommand::Ptr cmdCall(new BotCommand);
        cmdCall->command = "call";
        cmdCall->description = "Уравнять ставку";
        commands.push_back(cmdCall);

        BotCommand::Ptr cmdRaise(new BotCommand);
        cmdRaise->command = "raise";
        cmdRaise->description = "Повысить ставку";
        commands.push_back(cmdRaise);

        BotCommand::Ptr cmdFold(new BotCommand);
        cmdFold->command = "fold";
        cmdFold->description = "Сбросить карты";
        commands.push_back(cmdFold);

        BotCommand::Ptr cmdCheck(new BotCommand);
        cmdCheck->command = "check";
        cmdCheck->description = "Не делать ставку";
        commands.push_back(cmdCheck);

        BotCommand::Ptr cmdBet(new BotCommand);
        cmdBet->command = "bet";
        cmdBet->description = "Сделать ставку";
        commands.push_back(cmdBet);

        BotCommand::Ptr cmdStats(new BotCommand);
        cmdStats->command = "stats";
        cmdStats->description = "Информация об игроках";
        commands.push_back(cmdStats);

        // Устанавливаем команды
        bot.getApi().setMyCommands(commands);
        
        printf("Команды бота успешно зарегистрированы\n");
    } catch (TgException& e) {
        std::cerr << "Ошибка установки команд бота: " << e.what() << std::endl;
    }
}

void MyBot::skipPendingUpdates(Bot& bot)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    try {
        int lastUpdateId = 0;
        bool hasUpdates = true;
        
        while (hasUpdates) {
            // Получаем пачку обновлений (максимум 100 за раз)
            auto updates = bot.getApi().getUpdates(lastUpdateId, 10, 0);
            
            if (updates.empty()) {
                hasUpdates = false;
            } else {
                // Обновляем ID последнего обработанного сообщения
                lastUpdateId = updates.back()->updateId + 1;
                printf("Skipped %zu pending updates\n", updates.size());
            }
        }
        
        // Фиксируем последний ID как обработанный
        bot.getApi().getUpdates(lastUpdateId, 1, 0);
        printf("All pending updates skipped\n");
    } catch (const std::exception& e) {
        fprintf(stderr, "Error skipping updates: %s\n", e.what());
    }
}

void MyBot::run()
{
    printf("%s\n", __PRETTY_FUNCTION__);
    try {
        skipPendingUpdates(bot);
        TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (std::exception &e) {
        printf("%s\n", e.what());
        fprintf(stderr  , "Произошла ошибка и бот закрылся");
    }
}

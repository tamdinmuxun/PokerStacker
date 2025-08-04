#include "headers/mybot.h"

std::unordered_map<int64_t, std::shared_ptr<Player>> MyBot::players;
std::unordered_map<std::string, std::shared_ptr<Room>> MyBot::rooms;
Database MyBot::db(conn_str);

void MyBot::regPlayer(Bot &bot, int64_t user_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto player = MyBot::players[user_id] = std::make_shared<Player>(&bot, user_id);
    player->sendMessage("Введите Ваше имя:");
    player->setState(PlayerState::NAME);
}

std::pair<std::shared_ptr<Player>, std::shared_ptr<Room>> MyBot::getPlayerRoom(Bot &bot,
        int64_t user_id,
        const std::string &room_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);

    printf("Trying to retrieve player\n");
    if (MyBot::players.count(user_id)) {
        printf("Player in cache\n");
        auto player = MyBot::players[user_id];
        printf("Is nullptr: %d\n", (player==nullptr));
        if (!room_id.empty()) {
            if (MyBot::rooms.count(room_id)) {
                return {player, rooms[room_id]};
            } else {
                printf("Trying to find room in database\n");
                auto [owner_id, room] = db.getRoom(room_id);
                if (!room) {
                    printf("No such room in database\n");
                } else {
                    room->setOwner(player);
                    db.updateRoomOwner(room_id, player->getId());
                }
                return {player, room};
            }
        }
        auto room = player ? player->getRoom() : nullptr;
        return {player, room};
    } else {
        printf("No player in cache\nRetrieving player from database\n");
        try {
            auto player = MyBot::db.getPlayer(user_id);
            printf("Retrieved player from database\n");
            if (!player) {
                printf("No such player in database\nTrying to register one\n");
                bot.getApi().sendMessage(user_id, "Зарегестрируйтесь и попробуйте еще раз.");
                MyBot::regPlayer(bot, user_id);
                return {nullptr, nullptr};
            }
            player->setBot(&bot);
            MyBot::players[user_id] = player;
            return {player, nullptr};
        } catch (std::exception &obj) {
            printf("%s\n", obj.what());
        }
        return {nullptr, nullptr};
    }
}

void MyBot::handleStart(Bot &bot, int64_t user_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, room] = getPlayerRoom(bot, user_id);
    
    if (!player) {
        printf("No player in database\nRegistering\n");
        regPlayer(bot, user_id);
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
void MyBot::handleJoin(Bot &bot, int64_t user_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, room] = getPlayerRoom(bot, user_id);
    if (player){
        player->setState(PlayerState::ID);
        player->sendMessage("Введите идентификатор комнаты:");
    } else {
        bot.getApi().sendMessage(user_id, "Зарегестрируйтесь!");
    }
}

void MyBot::handleCreate(Bot &bot, int64_t user_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, r] = getPlayerRoom(bot, user_id);
    printf("Player found\nplayer id %lld\n user_id %lld\n", player->getId(), user_id);
    std::shared_ptr<Room> room = std::make_shared<Room>(player);
    MyBot::rooms[room->getId()] = room;
    printf("Room created\n");
    // TODO CREATE ROOM AT /SAVE COMMAND 
    // db.createRoom(room->getId(), user_id);

    player->setRoom(room);
    player->setState(PlayerState::CHIPS);
    printf("room set\n");
    player->sendMessage("Введите начальное количество фишек для каждого игрока:");
    printf("message sent\n");
}

void MyBot::handleLeave(Bot &bot, int64_t user_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto player = MyBot::players[user_id];

    if (player == nullptr) return; // ERROR PASS

    if (player->getState() == PlayerState::WAITING || player->getState() == PlayerState::GAMBLING) {
        auto room = MyBot::players[user_id]->getRoom();
        room->removePlayer(MyBot::players[user_id]);

        if (room->empty()) {
            MyBot::rooms.erase(room->getId());
        }
    } else {
        player->sendMessage("Вы не состоите ни в одной комнате");
    }
}

void MyBot::handleGamble(Bot &bot, int64_t user_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, room] = getPlayerRoom(bot, user_id);
    if (!player) {
        bot.getApi().sendMessage(user_id, "Зарегестрируйтесь с помощью команды /start!");
        return;
    }
    try {
        if (!room) {
            player->sendMessage("Вы не состоите ни в одной комнате!\nПрисоединиться /join\nСоздать /create");
            return;
        }
        room->updatePlayers(db);
        room->startGame();
    } catch (NotEnough &) {
        player->sendMessage("К сожалению, Вы не можете играть в гордом одиночестве(.");
    } catch (std::exception &e) {
        player->sendMessage("Error! Can't start game.");
        printf("%s\n", e.what());
    }
}

void MyBot::handleCall(Bot &bot, int64_t user_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, room] = getPlayerRoom(bot, user_id);
    if (!player) return;  // ERROR PASS

    if (!room) {
        player->sendMessage("Вы не состоите ни в одной комнате!");
    }

    player->makeBet(room->getCurrentBet());

    room->betting();
}

void MyBot::handleRaise(Bot &bot, int64_t user_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, room] = getPlayerRoom(bot, user_id);
    if (player == nullptr) return;  // ERROR PASS

    if (!room) {
        player->sendMessage("Вы не состоите ни в одной комнате!");
    }

    player->sendMessage(std::format("Введите ставку от {} до {}.", room->getCurrentBet() + 1, player->getChips()));
    player->setState(PlayerState::RAISING);
}

void MyBot::handleFold(Bot &bot, int64_t user_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, room] = getPlayerRoom(bot, user_id);
    if (!player) return;  // ERROR PASS

    if (!room) {
        player->sendMessage("Вы не состоите ни в одной комнате!");
    }
    printf("folding\n");
    fflush(stdout);
    player->setFold(true);

    room->betting();
}

void MyBot::handleStats(Bot &bot, int64_t user_id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, room] = getPlayerRoom(bot, user_id);
    if (!player) return;  // ERROR PASS
    room->stats();
}


void MyBot::handleName(Bot &bot, int64_t user_id, const std::string &name)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, room] = getPlayerRoom(bot, user_id);
    if (!player) return;  // ERROR PASS
    player->setName(name);
    player->setState(PlayerState::IDLE);

    db.createPlayer(user_id, name);

    player->sendMessage("Если хотите создать новую комнату, отправьте команду /create.\n\
Если хотите присоединиться к комнате, отправьте команду /join");
}

void MyBot::handleId(Bot &bot, int64_t user_id, const std::string &id)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    auto [player, room] = getPlayerRoom(bot, user_id, id);
    if (!player) return;  // ERROR PASS
    if (room) {
        if (room->addPlayer(player)) {
            MyBot::players[user_id]->setRoom(MyBot::rooms[id]);
            bot.getApi().sendMessage(user_id, "Комната найдена! Вы успешно присоединились!");
            MyBot::players[user_id]->setState(PlayerState::WAITING);
        }
    } else {
        bot.getApi().sendMessage(user_id, "Неверный идентификатор комнаты, либо ее не существует.");
    }
}

void MyBot::handleChips(Bot &bot, int64_t user_id, const std::string &text)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    printf("chips\n");
    try {
        if (MyBot::players[user_id] == nullptr) return;  // ERROR PASS

        int chips = std::strtol(text.c_str(), nullptr, 10);
        if (MyBot::players[user_id] == nullptr) return;  // ERROR PASS
        auto room = MyBot::players[user_id]->getRoom();

        if (room == nullptr) return;  // ERROR PASS
        db.updateRoomChips(room->getId(), chips);
        room->setChips(chips);
        bot.getApi().sendMessage(user_id, "Комната создана! Идентификатор: " + room->getId());
        room->addPlayer(MyBot::players[user_id]);
        MyBot::players[user_id]->setState(PlayerState::WAITING);
    } catch (std::exception &e) {
        printf("%s\n", e.what());
        bot.getApi().sendMessage(user_id, "Вы ввели не число, попробуйте еще раз.");
    }
}

void MyBot::handleRaising(Bot &bot, int64_t user_id, const std::string &text)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    try {
        int bet = std::strtol(text.c_str(), nullptr, 10);
        auto [player, room] = getPlayerRoom(bot, user_id);
        if (!player || !room) return;  // ERROR PASS

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
        bot.getApi().sendMessage(user_id, "Вы ввели число меньше текущей ставки.");
    } catch (TooMuch &) {
        bot.getApi().sendMessage(user_id, "Вы не можете поставить больше чем у вас есть.");
    } catch (...) {
        bot.getApi().sendMessage(user_id, "Вы ввели не число, попробуйте еще раз.");
    }
}

void MyBot::handleWinner(Bot &bot, int64_t user_id, const std::string &text)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    try {
        std::istringstream split(text);
        std::vector<int> winners;
        int i{};
        while (split >> i) {
            winners.push_back(--i);
        }

        auto [player, room] = getPlayerRoom(bot, user_id);
        if (player == nullptr) return;  // ERROR PASS
        
        if (room == nullptr) {
            player->sendMessage("Вы не состоите ни в одной комнате.");
            return;
        }

        room->distributePot(winners);
    } catch (std::exception &e) {
        bot.getApi().sendMessage(user_id, "Вы ввели не число, попробуйте еще раз.");
        printf("%s\n", e.what());
    }
}
void MyBot::handleMessage(Bot &bot, Message::Ptr message)
{
    printf("%s\n", __PRETTY_FUNCTION__);

    int64_t user_id = message->from->id;
    std::string text = message->text;
    printf("%s\n", text.c_str());
    PlayerState state = MyBot::players.count(user_id) ? MyBot::players[user_id]->getState() : PlayerState::IDLE;
    printf("message\n");
    switch (state) {
    case PlayerState::NAME:
        handleName(bot, user_id, text);
        break;
    case PlayerState::ID:
        handleId(bot, user_id, text);
        break;
    case PlayerState::CHIPS:
        handleChips(bot, user_id, text);
        break;
    case PlayerState::RAISING:
        handleRaising(bot, user_id, text);
        break;
    case PlayerState::WINNER:
        handleWinner(bot, user_id, text);
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

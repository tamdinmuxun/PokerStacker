#ifndef FWD_DECL
#define FWD_DECL

#include <cstdio>
#include <chrono>
#include <exception>
#include <random>
#include <string>
#include <tgbot/tgbot.h>

using namespace TgBot;

class Player;
class Room;
class MyBot;
class Pot;

// ERROR CLASSES
// TO-DO INHERIT FROM STD::EXCEPTION OR SOMETHING
class NotEnough {};
class TooMuch {};

constexpr int SMALL_BLIND = 5;
constexpr int BIG_BLIND = 10;
const std::string conn_str{"dbname=pokerdb host=localhost"};

extern std::mt19937 rng;
extern std::uniform_int_distribution<int> rnd;

std::string generateRoomId();
#endif

#ifndef FWD_DECL
#define FWD_DECL

#include <cstdio>
#include <chrono>
#include <random>
#include <string>
#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;

class Player;
class Room;
class MyBot;

// ERROR CLASSES
// TO-DO INHERIT FROM STD::EXCEPTION OR SOMETHING
class NotEnough {};
class TooMuch {};

constexpr int SMALL_BLIND = 5;
constexpr int BIG_BLIND = 10;
const string conn_str{"dbname=pokerdb host=localhost"};

extern mt19937 rng;
extern uniform_int_distribution<int> rnd;

string generateRoomId();
#endif

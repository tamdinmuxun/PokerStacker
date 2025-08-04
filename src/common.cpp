#include "headers/common.h"

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
std::uniform_int_distribution<int> rnd(0, static_cast<int>(1e9));

std::string generateRoomId()
{
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string id;
    for (int i = 0; i < 6; ++i) {
        id += chars[rnd(rng) % chars.size()];
    }
    return id;
}

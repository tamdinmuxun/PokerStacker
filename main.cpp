#include "mybot.h"

using namespace TgBot;


int main(int argc, char **argv)
{
    printf("start main\n");
    fflush(stdout);

    std::string token;
    if (argc == 2) {
        token = std::string(argv[1]);
    } else {
        fprintf(stderr, "Error: no token");
        exit(1);
    }
    MyBot bot(token);
    printf("bot initialized\n");
    bot.run();
}

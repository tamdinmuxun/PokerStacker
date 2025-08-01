#include "mybot.h"

using namespace TgBot;


int main()
{
    printf("start main\n");
    fflush(stdout);

    std::string token(getenv("TOKEN"));
    if (token == "") {
        printf("No bot token variable in environment\n");
        exit(1);
    }
    MyBot bot(token);
    printf("bot initialized\n");
    
    signal(SIGINT, [](int s) {
        printf("SIGINT got\n");
        exit(0);
    });

    bot.run();
}

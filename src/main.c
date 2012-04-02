#include <stdio.h>
#include <stdlib.h>
#include "bot.h"

void print_usage(const char *argv0) {
    printf("usage: %s <host> <port>\n", argv0);
}

int main(int argc, char **argv) {
    if(argc != 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *hostname = argv[1];
    const char *port = argv[2];

    bot *b = bot_new("enfin", "enfin", "An IRC bot in C", hostname, port);
    bot_run(b);

    return EXIT_SUCCESS;
}

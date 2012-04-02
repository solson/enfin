#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "bot.h"

bot *bot_new(const char *nick, const char *username, const char *realname,
        const char *hostname, const char *port) {
    bot *b = malloc(sizeof(bot));

    b->nick = strdup(nick);
    b->username = strdup(username);
    b->realname = strdup(realname);
    b->hostname = strdup(hostname);
    b->port = strdup(port);

    return b;
}

void bot_free(bot *b) {
    close(b->sock);
    free(b->nick);
    free(b->username);
    free(b->realname);
    free(b->hostname);
    free(b->port);
    free(b);
}

void bot_connect(bot *b) {
    int status;
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    status = getaddrinfo(b->hostname, b->port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    struct addrinfo *p;
    for(p = res; p != NULL; p = p->ai_next) {
        b->sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(b->sock == -1) {
            perror("error opening socket");
            continue;
        }

        status = connect(b->sock, res->ai_addr, res->ai_addrlen);
        if(status == -1) {
            perror("error connecting");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "error: failed to connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
}

void bot_register(bot *b) {
    bot_sendf(b, "NICK %s", b->nick);
    bot_sendf(b, "USER %s 0 * :%s", b->username, b->realname);
}

void bot_postregister(bot *b) {
    bot_sendf(b, "JOIN #bots");
    bot_sendf(b, "PRIVMSG #bots :It works!");
}

void bot_run(bot *b) {
    bot_connect(b);
    bot_register(b);

    while(true) {
        char buf[513];
        int bytesread = recv(b->sock, buf, 513, 0);
        buf[bytesread] = '\0';
        printf("%s", buf);

        if(NULL != strstr(buf, " 001 ")) {
            bot_postregister(b);
        }
    }
}

void bot_send_raw(bot *b, const char *msg) {
    int total = strlen(msg);
    int sent = 0;
    int left = total;

    while(sent < total) {
        int n = send(b->sock, msg + sent, left, 0);
        if(n == -1) {
            perror("error sending data");
            exit(EXIT_FAILURE);
        }

        sent += n;
        left -= n;
    }
}

void bot_sendf(bot *b, const char *format, ...) {
    va_list args;
    va_start(args, format);

    char *msg;
    vasprintf(&msg, format, args);

    bot_send_raw(b, msg);
    bot_send_raw(b, "\r\n");
}

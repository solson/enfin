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
#include <talloc.h>
#include "bot.h"
#include "ircmsg.h"

bot *bot_new(const char *nick, const char *username, const char *realname,
        const char *hostname, const char *port) {
    bot *b = talloc(NULL, bot);

    b->nick     = talloc_strdup(b, nick);
    b->username = talloc_strdup(b, username);
    b->realname = talloc_strdup(b, realname);
    b->hostname = talloc_strdup(b, hostname);
    b->port     = talloc_strdup(b, port);

    return b;
}

void bot_free(bot *b) {
    close(b->sock);
    talloc_free(b);
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
}

void bot_run(bot *b) {
    bot_connect(b);
    bot_register(b);

    char buf[513]; // extra space for nul when line is 512 bytes
    int leftover = 0;

    while(true) {
        int bytesread = recv(b->sock, buf + leftover, sizeof(buf) - leftover - 1, 0);

        if(bytesread == 0) {
            printf("remote host closed the connection\n");
            exit(EXIT_SUCCESS);
        } else if(bytesread == -1) {
            perror("error recieving data");
            exit(EXIT_FAILURE);
        }

        // start looking for \r\n at the end of leftover
        int i = leftover ? leftover - 1 : 0;
        // the index of the start of the current line
        int start = 0;

        // Look for \r\n line endings and handle the lines if found
        while(i < leftover + bytesread - 1) {
            if(buf[i] == '\r' && buf[i + 1] == '\n') {
                // Don't do anything with empty lines.
                if(start != i) {
                    buf[i] = '\0'; // End the string at the \r
                    bot_handle_raw(b, buf + start);
                }

                i += 2; // Skip the \r\n
                start = i; // Next line starts after
            } else {
                i++;
            }
        }

        leftover = leftover + bytesread - start;

        // If the buffer fills, the line is invalid IRC (messages must be
        // under 512 bytes)
        if(leftover >= sizeof(buf) - 1) {
            buf[sizeof(buf) - 1] = '\0';
            fprintf(stderr, "error: Line received was longer than the IRC "
                    "protocol allows. Showing first 512 bytes:\n%s\n", buf);
            exit(EXIT_FAILURE);
        }

        if(leftover > 0) {
            memmove(buf, buf + start, leftover);
        }
    }
}

void bot_send_raw(bot *b, const char *raw_msg) {
    int total = strlen(raw_msg);
    int sent = 0;
    int left = total;

    while(sent < total) {
        int n = send(b->sock, raw_msg + sent, left, 0);
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
    char *msg = talloc_vasprintf(NULL, format, args);
    va_end(args);

    bot_send_raw(b, msg);
    bot_send_raw(b, "\r\n");

    talloc_free(msg);
}

void bot_handle_raw(bot *b, const char *raw_msg) {
    printf(">> \"%s\"\n", raw_msg);

    ircmsg *m = ircmsg_parse_new(raw_msg);

    if(0 == strcmp(m->command, "001")) {
        bot_postregister(b);
    }

    ircmsg_free(m);
}

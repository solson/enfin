#ifndef __BOT_H__
#define __BOT_H__

typedef struct {
    // IRC user info
    char *nick;
    char *username;
    char *realname;

    // Connection info
    char *hostname;
    char *port; // number or service name recognized by getaddrinfo

    // TCP Socket
    int sock;
} bot;

bot *bot_new(const char *nick, const char *username, const char *realname,
        const char *hostname, const char *port);
void bot_free(bot *b);

void bot_connect(bot *b);
void bot_register(bot *b);
void bot_postregister(bot *b);
void bot_run(bot *b);

void bot_send_raw(bot *b, const char *msg);

#endif /* __BOT_H__ */

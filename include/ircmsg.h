#ifndef __IRCMSG_H__
#define __IRCMSG_H__

// structure representing one IRC message or line
typedef struct {
    char *prefix;     // optional (NULL if not present)
    char *command;    // an IRC command or numeric
    char *params[15]; // the parameters for the command or numeric
    int param_count;  // 0..15 (the IRC RFC specifies a max of 15 params)
} ircmsg;

ircmsg *ircmsg_new(const char *prefix, const char *command,
        const char **params, int param_count);
ircmsg *ircmsg_parse_new(const char *raw_msg);
void ircmsg_free(ircmsg *msg);

char *ircmsg_render(ircmsg *msg);

#endif /* __IRCMSG_H__ */

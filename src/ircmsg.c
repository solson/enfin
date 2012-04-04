#include <stdlib.h>
#include <string.h>
#include <talloc.h>
#include "ircmsg.h"

ircmsg *ircmsg_new(const char *prefix, const char *command,
        const char **params, int param_count) {
    ircmsg *m = talloc(NULL, ircmsg);

    m->prefix      = talloc_strdup(m, prefix);
    m->command     = talloc_strdup(m, command);
    m->param_count = param_count;

    for(int i = 0; i < param_count; i++)
        m->params[i] = talloc_strdup(m, params[i]);

    return m;
}

ircmsg *ircmsg_parse_new(const char *raw_msg) {
    ircmsg *m = talloc(NULL, ircmsg);
    const char *p = raw_msg;
    const char *end;

    // Parse the prefix, if it is present
    if(p[0] == ':') {
        p++;

        end = strchr(p, ' ');
        if(!end) { return NULL; }

        m->prefix = talloc_strndup(m, p, end - p);

        // Skip past the spaces after the prefix
        p = end;
        while(*p == ' ') { p++; }
    } else {
        m->prefix = NULL;
    }

    // Parse the command or numeric
    end = strchr(p, ' ');
    if(!end) { end = p + strlen(p); }

    m->command = talloc_strndup(m, p, end - p);
    p = end;

    // Parse the command parameters
    m->param_count = 0;

    while(m->param_count < 15) {
        // Stop if we hit the end of the message
        if(*p == '\0') { break; }

        // Skip spaces
        while(*p == ' ') { p++; }

        // The last param begins with a colon and may include spaces
        if(*p == ':') {
            m->params[m->param_count] = talloc_strdup(m, p + 1);
            m->param_count++;
            break;
        }

        end = strchr(p, ' ');
        if(!end) { end = p + strlen(p); }

        m->params[m->param_count] = talloc_strndup(m, p, end - p);
        m->param_count++;
        p = end;
    }

    return m;
}

void ircmsg_free(ircmsg *msg) {
    talloc_free(msg);
}

char *ircmsg_render(ircmsg *msg) {
    char *str = talloc_strdup(NULL, "");

    if(msg->prefix)
        str = talloc_asprintf_append(str, ":%s ", msg->prefix);

    str = talloc_asprintf_append(str, "%s ", msg->command);

    for(int i = 0; i < msg->param_count; i++) {
        str = talloc_asprintf_append(str, 
                (i == msg->param_count - 1 ? ":%s" : "%s "),
                msg->params[i]);
    }

    return str;
}

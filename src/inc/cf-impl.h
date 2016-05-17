#ifndef CF_IMPL_H

#ifndef CF_H
#error "Require cf.h"
#endif

#define CF_IMPL_H 1

struct cf {
    int err;
    int state;
    enum cclass_set cclass;
};

enum state {
    S_START,
    S_START_DQUOTE,
    S_DQUOTE_ESCAPE,
    S_START_SLASH,
    S_SLASH_STAR,
    S_SLASH_STAR_STAR,
    S_START_SQUOTE,
    S_SQUOTE_ESCAPE,
    S_INCHAR,
    S_COMMENT_EOL,	// Alias S_SLASH_SLASH
    S_EOF,
};

extern const char *decode_state(int state);

#endif /* CF_IMPL_H */

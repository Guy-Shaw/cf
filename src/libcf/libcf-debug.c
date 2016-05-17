#include <stdarg.h>
    // Import sprintf()
#include <stdio.h>
    // Import constant EOF
    // Import sprintf()

#include <cf.h>
#include <cf-impl.h>

const char *
decode_cclass(int ccl)
{
    static char ubuf[20];
    const char *ccl_str;

    switch (ccl) {
    case CC_UNDEF:         ccl_str = "UNDEF";         break;
    case CC_CODE:          ccl_str = "CODE";          break;
    case CC_OUTER_STRING:  ccl_str = "OUTER_STRING";  break;
    case CC_INNER_STRING:  ccl_str = "INNER_STRING";  break;
    case CC_OUTER_CHAR:    ccl_str = "OUTER_CHAR";    break;
    case CC_INNER_CHAR:    ccl_str = "INNER_CHAR";    break;
    case CC_OUTER_COMMENT: ccl_str = "OUTER_COMMENT"; break;
    case CC_INNER_COMMENT: ccl_str = "INNER_COMMENT"; break;
    default:
        sprintf(ubuf, "Unknown CCL#%d", ccl);
        ccl_str = (const char *)ubuf;
    }
    return (ccl_str);
}

const char *
decode_state(int state)
{
    static char ubuf[20];
    const char *state_str;

    switch (state) {
    case S_START:           state_str = "START";            break;
    case S_START_DQUOTE:    state_str = "START_DQUOTE";     break;
    case S_DQUOTE_ESCAPE:   state_str = "DQUOTE_ESCAPE";    break;
    case S_START_SLASH:     state_str = "START_SLASH";      break;
    case S_SLASH_STAR:      state_str = "SLASH_STAR";       break;
    case S_SLASH_STAR_STAR: state_str = "SLASH_STAR_STAR";  break;
    case S_START_SQUOTE:    state_str = "START_SQUOTE";     break;
    case S_SQUOTE_ESCAPE:   state_str = "SQUOTE_ESCAPE";    break;
    case S_INCHAR:          state_str = "INCHAR";           break;
    case S_COMMENT_EOL:     state_str = "COMMENT_EOL";      break;
    case S_EOF:             state_str = "EOF";              break;
    default:
        sprintf(ubuf, "Unknown #%d", state);
        state_str = (const char *)ubuf;
    }
    return (state_str);
}

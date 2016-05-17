
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <ctype.h>
    // Import isprint()
#include <errno.h>
    // Import var errno
#include <getopt.h>
    // Import getopt_long()
#include <stdarg.h>
    // Import fprintf()
    // Import snprintf()
#include <stdbool.h>
    // Import constant false
    // Import constant true
    // Import type bool
#include <stddef.h>
    // Import constant NULL
#include <stdio.h>
    // Import constant EOF
    // Import fgetc()
    // Import fopen()
    // Import fprintf()
    // Import fputc()
    // Import fputs()
    // Import putchar()
    // Import snprintf()
    // Import type FILE
    // Import var stderr
    // Import var stdout
#include <stdlib.h>
    // Import exit()
    // Import malloc()
#include <unistd.h>
    // Import close()
    // Import exit()
    // Import getopt_long()
    // Import type size_t
#include <string.h>
    // Import strncmp()

#include <cscript.h>
#include <cf.h>

const char *program_path;
const char *program_name;

size_t filec;               // Count of elements in filev
char **filev;               // Non-option elements of argv

bool verbose = false;
bool debug   = false;

static bool preserve_lines = false;

FILE *errprint_fh = NULL;
FILE *dbgprint_fh = NULL;

extern const char *decode_cclass(int ccl);

static void
ccv_flush(ccv_t *ccv, int cclset)
{
    size_t len;
    size_t pos;

    len = ccv->len;
    if (len == 0) {
        return;
    }

    for (pos = 0; pos < len; ++pos) {
        int ccl;
        int chr;
        bool show;

        ccl = ccv->array[pos].ccl;
        chr = ccv->array[pos].chr;
        dbg_printf("    chr=%d, ccl=%s\n", chr, decode_cclass(ccl));

        if (chr == EOF) {
            break;
        }
        show = (ccl & cclset) != 0 || (preserve_lines && chr == '\n');
        if (show) {
            if (chr == '\n') {
                putchar(chr);
            }
            else {
                char dbuf[8];

                show_char_r(dbuf, sizeof (dbuf), chr);
                fputs(dbuf, stdout);
            }
        }
    }

    ccv->len = 0;
}

static struct option long_options[] = {
    {"help",           no_argument,       0,  'h'},
    {"version",        no_argument,       0,  'V'},
    {"verbose",        no_argument,       0,  'v'},
    {"debug",          no_argument,       0,  'd'},
    {"preserve-lines", no_argument,       0,  'p'},
    {"show",           required_argument, 0,  's'},
    {0, 0, 0, 0}
};

static const char usage_text[] =
    "Options:\n"
    "  --help|-h|-?         Show this help message and exit\n"
    "  --version            Show version information and exit\n"
    "  --verbose|-v         verbose\n"
    "  --debug|-d           debug\n"
    "  --preserve-lines|-p  Always print newlines, even inside comments\n"
    "                       or strings that otherwise would not be printed.\n"
    "                       will not change.\n"
    "  --show=<parts>       Specify what parts to show.\n"
    "\n"
    "Where <parts> is a comma-separated list of syntax classes.\n"
    "Syntax classes are one of the classes named below, a class\n"
    "name prefixed with '!' or '-' to remove that syntax class\n"
    "from the set.\n"
    "    *,all,0,null,empty,code\n"
    "    comment, outer-comment, inner-comment,\n"
    "    string, outer-string, inner-string,\n"
    "    char, outer-char, inner-char,\n"
    "\n"
    "    comment is { outer-comment \\union inner-comment },\n"
    "    string  is { outer-string \\union inner-string },\n"
    "    char    is { outer-char \\union inner-char },\n"
    "    * and all are synonyms for the union of all syntax classes\n"
    "    0, null, empty are synonyms for the empty set\n"
    "\n"
    "    The empty set is useful when it is more convenient to express\n"
    "    what parts we want, from scratch, rather that start with 'all',\n"
    "    then take away what we do not want.\n"
    "    'code' is like a macro, meaning:\n"
    "        'all,-comment,-inner-string,-inner-char'\n"
    "    It will strip all comments and the contents of strings\n"
    "    (in double quotes) and char constants (in single quotes),\n"
    "    but leave the quote marks, themselves.\n"
    "\n"
    "The --preserve-lines option is used to strip the contents of\n"
    "comments or strings while not disturbing source code line numbers.\n"
    "\n"
    "Example:\n"
    "  --show=all,-comment\n"
    "      strips comments.\n"
    "\n"
    "  --show=comments\n"
    "      filters out everything but comments\n"
    "\n"
    "  --show=code\n"
    "      strips comments, inner-string, and inner-char\n"
    "      but leaves the single quote marks and double-quote marks\n"
    ;

static const char version_text[] =
    "0.1\n"
    ;

static const char copyright_text[] =
    "Copyright (C) 2016 Guy Shaw\n"
    "Written by Guy Shaw\n"
    ;

static const char license_text[] =
    "License GPLv3+: GNU GPL version 3 or later"
    " <http://gnu.org/licenses/gpl.html>.\n"
    "This is free software: you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n"
    ;

static void
fshow_program_version(FILE *f)
{
    fputs(version_text, f);
    fputc('\n', f);
    fputs(copyright_text, f);
    fputc('\n', f);
    fputs(license_text, f);
    fputc('\n', f);
}

static void
show_program_version(void)
{
    fshow_program_version(stdout);
}

static void
usage(void)
{
    eprintf("usage: %s [ <options> ]\n", program_name);
    eprint(usage_text);
}

static inline bool
is_long_option(const char *s)
{
    return (s[0] == '-' && s[1] == '-');
}

static inline char *
vischar_r(char *buf, size_t sz, int c)
{
    if (isprint(c)) {
        buf[0] = c;
        buf[1] = '\0';
    }
    else {
        snprintf(buf, sz, "\\x%02x", c);
    }
    return (buf);
}

int
ccl_stream(FILE *f, int cclset)
{
    ccv_t *ccv = ccv_new();
    cf_t *ctx = cf_new(cclset);

    while (true) {
        int rv;
        int c;

        c = fgetc(f);
        rv = cf_next(ctx, ccv, c);
        if (rv != 0 && rv != EOF) {
            fprintf(errprint_fh, "ERR %d\n", rv);
            exit(2);
        }
        ccv_flush(ccv, cclset);
        if (c == EOF) {
            break;
        }
    }

    return (0);
}

int
filev_ccl(int cclset)
{
    size_t fnr;

    for (fnr = 0; fnr < filec; ++fnr) {
        FILE *f;
        int rv;
        int close_rv;

        f = fopen(filev[fnr], "r");
        if (f == NULL) {
            int err = errno;
            fprintf(errprint_fh, "fopen('%s', \"r\") failed\n", filev[fnr]);
            fprintf(errprint_fh, "  errno=%d\n", err);
            return (err);
        }

        rv = ccl_stream(f, cclset);
        close_rv = fclose(f);
        if (rv) {
            fprintf(errprint_fh, "ccl_stream('%s') failed.\n", filev[fnr]);
            return (rv);
        }
        if (close_rv) {
            fprintf(errprint_fh, "fclose('%s') failed.\n", filev[fnr]);
            return (rv);
        }
    }

    return (0);
}

struct ccl_name {
    const char *name;
    int ccl;
};

#define CC_STRING  (CC_OUTER_STRING | CC_INNER_STRING)
#define CC_CHAR    (CC_OUTER_CHAR | CC_INNER_CHAR)
#define CC_COMMENT (CC_OUTER_COMMENT | CC_INNER_COMMENT)
#define CC_ALL (CC_CODE | CC_STRING | CC_CHAR | CC_COMMENT)

static struct ccl_name ccl_name_table[] = {
    { "all", CC_ALL },
    { "*",   CC_ALL },
    { "0",     0 },
    { "null",  0 },
    { "empty", 0 },
    { "code",  CC_CODE },
    { "comment", CC_COMMENT },
    { "outer-comment", CC_OUTER_COMMENT },
    { "inner-comment", CC_INNER_COMMENT },
    { "string", CC_STRING },
    { "outer-string", CC_OUTER_STRING },
    { "inner-string", CC_INNER_STRING },
    { "char",       CC_CHAR },
    { "outer-char", CC_OUTER_CHAR },
    { "inner-char", CC_INNER_CHAR },
    { 0, 0 }
};

int
lookup_ccl(const char *s, size_t len)
{
    struct ccl_name *t;

    for (t = ccl_name_table; t->name; ++t) {
        if (strncmp(s, t->name, len) == 0) {
            return (t->ccl);
        }
    }
    return (-1);
}

int
parse_ccl_set(const char *cclset_str)
{
    const char *p;
    int cclset = -1;

    p = cclset_str;
    while (*p) {
        const char *e;
        int ccl;
        int sgn = 1;

        if (*p == '-' || *p == '!') {
            sgn = -1;
            ++p;
        }
        e = p;
        while (*e && (isalpha(*e) || *e == '-')) {
            ++e;
        }
        ccl = lookup_ccl(p, e - p);
        if (ccl == -1) {
            fprintf(errprint_fh, "Unknown syntax class name, '");
            fshow_strn(errprint_fh, p, e - p);
            fprintf(errprint_fh, "'.\n");
            return (-1);
        }
        if (cclset == -1) {
            if (sgn < 0) {
                cclset = ~ccl;
            }
            else {
                cclset = ccl;
            }
        }
        else {
            if (sgn < 0) {
                cclset &= ~ccl;
            }
            else {
                cclset |= ccl;
            }
        }

        p = e;
        if (*p) {
            if (*p != ',') {
                fprintf(errprint_fh, "Syntax error.  Expecting comma.\n");
                return (-1);
            }
            ++p;
        }
    }

    return (cclset);
}


int
main(int argc, char **argv)
{
    extern char *optarg;
    extern int optind, opterr, optopt;
    int option_index;
    int err_count;
    int optc;
    int rv;
    int cclset;

    set_eprint_fh();
    program_path = *argv;
    program_name = sname(program_path);
    option_index = 0;
    err_count = 0;
    opterr = 0;

    while (true) {
        int this_option_optind;

        if (err_count > 10) {
            eprintf("%s: Too many option errors.\n", program_name);
            break;
        }

        this_option_optind = optind ? optind : 1;
        optc = getopt_long(argc, argv, "+hVdvpk:s:", long_options, &option_index);
        if (optc == -1) {
            break;
        }

        rv = 0;
        if (optc == '?' && optopt == '?') {
            optc = 'h';
        }

        switch (optc) {
        case 'V':
            show_program_version();
            exit(0);
            break;
        case 'h':
            fputs(usage_text, stdout);
            exit(0);
            break;
        case 'd':
            debug = true;
            set_debug_fh(NULL);
            break;
        case 'v':
            verbose = true;
            break;
        case 'p':
            preserve_lines = true;
            break;
        case 's':
            cclset = parse_ccl_set(optarg);
            if (cclset == -1) {
                exit(2);
            }
            break;
        case '?':
            eprint(program_name);
            eprint(": ");
            if (is_long_option(argv[this_option_optind])) {
                eprintf("unknown long option, '%s'\n",
                    argv[this_option_optind]);
            }
            else {
                char chrbuf[10];
                eprintf("unknown short option, '%s'\n",
                    vischar_r(chrbuf, sizeof (chrbuf), optopt));
            }
            ++err_count;
            break;
        default:
            eprintf("%s: INTERNAL ERROR: unknown option, '%c'\n",
                program_name, optopt);
            exit(2);
            break;
        }
    }

    verbose = verbose || debug;

    if (argc != 0) {
        filec = (size_t) (argc - optind);
        filev = argv + optind;
    }
    else {
        filec = 0;
        filev = NULL;
    }

    if (verbose) {
        fshow_str_array(errprint_fh, filec, filev);
    }

    if (verbose && optind < argc) {
        eprintf("non-option ARGV-elements:\n");
        while (optind < argc) {
            eprintf("    %s\n", argv[optind]);
            ++optind;
        }
    }

    if (err_count != 0) {
        usage();
        exit(1);
    }

    if (filec == 0) {
        rv = ccl_stream(stdin, cclset);
    }
    else {
        rv = filev_probe(filec, filev);
        if (rv != 0) {
            exit(rv);
        }

        rv = filev_ccl(cclset);
    }

    if (rv != 0) {
        exit(rv);
    }

    exit(0);
}

#include <fcntl.h>
    // Import fstat()
#include <stddef.h>
    // Import constant NULL
#include <stdio.h>
    // Import fopen()
    // Import stderr()
    // Import stdout()
    // Import type FILE
    // Import var stderr
    // Import var stdout
#include <stdlib.h>
    // Import getenv()
#include <string.h>
    // Import memcmp()
#include <sys/stat.h>
    // Import fstat()
#include <sys/types.h>
    // Import fstat()
#include <unistd.h>
    // Import fstat()

extern FILE *errprint_fh;
extern FILE *dbgprint_fh;

struct fid {
    dev_t dev;
    ino_t ino;
};

/*
 * Compare the dev+ino of stdout and stderr
 *
 * Return:
 *     0  stderr (fd2) is the same dev+ino as stdout (fd1).
 *     1  stderr (fd2) is redirected to some place other than stdout (fd1).
 *    -1  some error occurred while trying to stat fd1 or fd2. 
 */
int
stderr_redirected(void)
{
    struct stat statbuf;
    struct fid stdout_id;
    struct fid stderr_id;
    int rv;

    rv = fstat(1, &statbuf);
    if (rv != 0) {
        return (-1);
    }
    stdout_id.dev = statbuf.st_dev;
    stdout_id.ino = statbuf.st_ino;
    rv = fstat(2, &statbuf);
    if (rv != 0) {
        return (-1);
    }
    rv = memcmp(&stderr_id, &stdout_id, sizeof (struct fid));
    rv = (rv != 0);
    return (rv);
}

void
set_debug_fh(const char *dbg_fname)
{
    if (dbg_fname == NULL) {
        dbgprint_fh = NULL;
        return;
    }

    if (dbg_fname[0] != '\0') {
        dbgprint_fh = fopen(dbg_fname, "w");
        return;
    }

    dbg_fname = getenv("DEBUG.ccomment");
    dbgprint_fh = fopen(dbg_fname, "w");
    if (dbgprint_fh) {
        return;
    }

    dbgprint_fh = fopen("/proc/fd/self/3", "w");
    if (dbgprint_fh) {
        return;
    }

    dbgprint_fh = errprint_fh;
    if (dbgprint_fh) {
        return;
    }
    dbgprint_fh = stderr;
}

void
set_eprint_fh(void)
{
    int rv;
    FILE *efh;

    rv = stderr_redirected();
    efh = (rv == 0) ? stdout : stderr;

    if (errprint_fh == NULL) {
        errprint_fh = efh;
    }
}

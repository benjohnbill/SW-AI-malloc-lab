/* bsize_probe.c — a missing (size) turns a call into a function address.
 * gcc -g -O0 -Wall bsize_probe.c -o bsize_probe            (cast: 0 warnings)
 * gcc -g -O0 -Wall -DNOCAST bsize_probe.c -o bsize_probe   (no cast: warning)
 */
#include <stdio.h>
#include <stddef.h>

static unsigned int bsize(size_t psize) { return 8 + (unsigned int)psize + 8; }

int main(void) {
    unsigned int blk = 1024;
#ifdef NOCAST
    if (blk >= bsize) puts("fit"); else puts("no fit");
#else
    if (blk >= (size_t)bsize) puts("fit"); else puts("no fit");
#endif
    printf("(size_t)bsize = %#zx\n", (size_t)bsize);
    printf("bsize(1000)   = %u\n", bsize(1000));
    return 0;
}

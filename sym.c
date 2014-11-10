/* Symbol table operations. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "size.h"
#include "die.h"
#include "mem.h"

/* Symbol table entry */
struct syment {
    char *sym; /* Name of the symbol */
    size_t off; /* Offset of its data word in the computer's word-addressable memory */
};

/* The symbol table, really just an array. Could be a tree or a
 * hash-table in a more demanding application. */
static struct syment *syms;

/* Count of symbols currently in the symbol table. */
static size_t nsym;

/*
 * xstrdup -- copy a C string in memory or die if out of memory
 *
 * src -- the C string (null-terminated string) to copy
 * return value -- a copy of the given string
 */
static char *xstrdup(char *src)
{
    size_t size;
    char *dst;

    size = strlen(src)+1;
    if(!(dst = malloc(size))) die("out of memory");
    memcpy(dst, src, size);
    return(dst);
}

/*
 * addsym -- add a symbol to the symbol table
 *
 * sym -- the name of the new symbol
 * off -- the offset of the symbol's data word in the computer's memory
 *
 * Doesn't check for duplicates. Dies if out of memory.
 */
void addsym(char *sym, size_t off)
{
    syms = realloc(syms, size_mul(size_add(nsym, 1), sizeof(struct syment)));
    if(!syms) die("out of memory");
    syms[nsym].sym = xstrdup(sym);
    syms[nsym].off = off;
    nsym++;
}

/*
 * printsymtab -- write the symbol table to standard output
 *
 * Writes the data word of each symbol whose data word lies in the
 * data area of the computer's memory.
 */
void printsymtab(void)
{
    struct syment *ent;
    size_t val;

    for(ent=syms; ent<syms+nsym; ent++)
    {
        if((ent->off >= dataoff) && (ent->off < dataoff+datasize))
        {
            val = getmem(ent->off);
            printf("%s(0x%zx) == 0x%zx (decimal %zd)\n",
                   ent->sym, ent->off, val, (ssize_t)val);
        }
    }
}

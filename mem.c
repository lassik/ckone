/* Represents the simulated computer's memory. */

#include <stdlib.h>

#include "size.h"
#include "die.h"
#include "mem.h"

size_t *mem;
size_t memsize;
size_t codeoff;
size_t codesize;
size_t dataoff;
size_t datasize;

/* 
 * addmem -- Add memory at the end of the address space of the
 * simulated computer.
 *
 * increment -- how many words to add
 *
 * error-exists if there isn't enough memory available for the host process
 */
void addmem(size_t increment)
{
    memsize = size_add(memsize, increment);
    if(!(mem = realloc(mem, size_mul(memsize, sizeof(size_t))))) die("out of memory");
}

/*
 * checkaddr -- error-exit if the given address is out of bounds
 *
 * addr -- the given address
 */
void checkaddr(size_t addr)
{
    if(addr < memsize) return;
    die("invalid memory address");
}

/*
 * getmem -- fetch a word from memory
 *
 * addr -- the address of the word
 */
size_t getmem(size_t addr)
{
    checkaddr(addr);
    return(mem[addr]);
}

/*
 * setmem -- store a word in memory
 *
 * addr -- the address in which the word is to be stored
 * word -- the word to be stored
 */
void setmem(size_t addr, size_t word)
{
    checkaddr(addr);
    mem[addr] = word;
}

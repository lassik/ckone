/* This module defines and provides access to the eight general
 * purpose registers of the TTK-91 computer. When we talk about
 * register numbers, we mean the ones encoded in the Ri and Rj fields
 * in TTK-91 instruction words. The TTK-91 computer's four control
 * registers are internal to the simulator module and are not dealt
 * with here. */

#include <stdlib.h>

#include "die.h"
#include "reg.h"

/* Value of each general-purpose register in the simulated computer */
size_t regs[8];

/* Table mapping register numbers to register names */
const char *regnames[8] = {"R0", "R1", "R2", "R3", "R4", "R5", "SP", "FP"};

/*
 * Register access
 */

/*
 * checkreg -- assert that the given register number is valid
 *
 * reg -- the register number
 */
void checkreg(size_t reg)
{
    if(reg < 8) return;
    die("invalid register number");
}

/*
 * getreg -- fetch the word in the register with the given number
 *
 * reg -- the register number
 *
 * Dies if the register number is invalid.
 */
size_t getreg(size_t reg)
{
    checkreg(reg);
    return(regs[reg]);
}

/*
 * setreg -- store the given word in the register with the given number
 *
 * reg -- the register number
 * word -- the word to store
 *
 * Dies if the register number is invalid.
 */
void setreg(size_t reg, size_t word)
{
    checkreg(reg);
    regs[reg] = word;
}

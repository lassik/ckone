/* This module defines and provides access to the eight general
 * purpose registers of the TTK-91 computer. When we talk about
 * register numbers, we mean the ones encoded in the Ri and Rj fields
 * in TTK-91 instruction words. The TTK-91 computer's four control
 * registers are internal to the simulator module and are not dealt
 * with here. */

/* Mnemonic for register number 6, the stack pointer register */
#define SP 6

/* Mnemonic for register number 7, the frame pointer register */
#define FP 7

/* Value of each general-purpose register in the simulated computer */
extern size_t regs[8];

/* Table mapping register numbers to register names */
extern const char *regnames[8];

void checkreg(size_t reg);
size_t getreg(size_t reg);
void setreg(size_t reg, size_t word);

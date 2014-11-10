/* The TTK-91 simulator proper. Whereas loading, instruction decoding
 * and memory management are done in other modules, this one actually
 * simulates the CPU logic. */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "size.h"
#include "die.h"
#include "mem.h"
#include "reg.h"
#include "insn.h"
#include "disasm.h"
#include "ckone.h"
#include "sim.h"

/* This macro gives the number of elements in xs, where xs is the name
 * of any fixed-size array whose size has been declared in the source
 * code. */
#undef  COUNTOF
#define COUNTOF(xs) (sizeof(xs)/sizeof(xs[0]))

/* Bitmasks for state register bits, for use with bitwise operations */
#define SR_L 2 /* Whether last numerical comparison produced "less than" */
#define SR_E 1 /* Whether last numerical comparison produced "equal to" */
#define SR_G 0 /* Whether last numerical comparison produced "greater than" */

/* TTK-91 control registers */
static size_t pc; /* Program counter */
static size_t ir; /* Instruction register */
static size_t tr; /* Temporary register */
static size_t sr; /* State register */

/* Nonzero if the HALT supervisor call has been issued. */
static int halted;

/*
 * Stack operations
 */

/* Note that stacks grow upward, not downward. Stack limits are not
 * mentioned anywhere so we can't guard against stack overflow or
 * stack underflow. */

/*
 * push -- push the given word on the given stack
 *
 * sp -- stack pointer register to use. Can be any of the eight
 * general purpose registers.
 * word -- the word to push on the stack
 */
static void push(size_t sp, size_t word)
{
    setreg(sp, getreg(sp)+1);
    setmem(getreg(sp), word);
}

/*
 * pop -- pop a word off the given stack
 *
 * sp -- stack pointer register to use. Can be any of the eight
 * general purpose registers.
 * return value -- the word popped off the stack
 */
static size_t pop(size_t sp)
{
    size_t word;

    word = getmem(getreg(sp));
    setreg(sp, getreg(sp)-1);
    return(word);
}

/*
 * State register operations
 */

/*
 * getsrbit -- get the given bit from the state register
 *
 * bit -- index of the bit to get, where 0 <= bit < SIZE_BIT
 * return value -- zero if the bit is false, nonzero otherwise
 */
static size_t getsrbit(size_t bit)
{
    return(!!(sr & (1<<bit)));
}

/*
 * setsrbit -- set the value of the given bit in the state register
 *
 * bit -- index of the bit to set, where 0 <= bit < SIZE_BIT
 * val -- the boolean value to set it to, where nonzero is true
 */
static void setsrbit(size_t bit, size_t val)
{
    if(val) sr |= (1<<bit); else sr &= ~(1<<bit);
}

/*
 * compare -- compare two words and update the state register accordingly
 *
 * a -- the left-hand-side word
 * b -- the right-hand-side word
 */
static void compare(size_t a, size_t b)
{
    setsrbit(SR_L, a<b);
    setsrbit(SR_E, a==b);
    setsrbit(SR_G, a>b);
}

/* 
 * I/O device implementations
 */

/*
 * input -- read a signed decimal integer from standard input
 *
 * return value -- the unsigned word representation of the integer read
 *
 * Dies on input error. Integer overflow checking not done.
 */
static size_t input(void)
{
    ssize_t ss;

    printf("Input: ");
    fflush(stdout);
    if(scanf("%zd", &ss)!=1) die("cannot read from standard input");
    if(ferror(stdin)) die("cannot read from standard input");
    if(verbose) printf("Received input: %zd\n", ss);
    return(ss);
}

/*
 * output -- write a signed decimal integer to standard output
 *
 * val -- the unsigned word representation of the integer to write
 */
static void output(size_t val)
{
    printf("Output: %zd\n", (ssize_t)val);
    fflush(stdout);
}

/* Table mapping port numbers to input device implementations (just C functions) */
static size_t (*intab[])(void) = {0, input, 0, 0, 0, 0, input};

/* Table mapping port numbers to output device implementations (just C functions) */
static void (*outtab[])(size_t) = {output, 0, 0, 0, 0, 0, 0, output};

/*
 * Supervisor call implementations
 */

/* Each takes as a parameter the stack pointer register to use. That
 * register is passed straight to the functions in the Stack
 * operations section of this module. */

static void svc_halt(size_t sp)
{
    printf("HALT\n");
    fflush(stdout);
    halted = 1;
}

static void svc_time(size_t sp)
{
    die("TIME supervisor call not implemented");
}

static void svc_date(size_t sp)
{
    die("DATE supervisor call not implemented");
}

static void svc_read(size_t sp)
{
    setmem(pop(sp), input());
}

static void svc_write(size_t sp)
{
    output(pop(sp));
}

/* Table mapping supervisor call numbers to their implementation (just
 * C functions). */
static void (*svctab[])(size_t) =
{
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, svc_halt,
    svc_read, svc_write, svc_time, svc_date,
};

/*
 * Fetch-decode-execute cycle
 */

/*
 * simulate -- execute the program one CPU instruction at a time until HALT
 */
void simulate(void)
{
    struct insn *insn;
    size_t reg;
    ssize_t tmp;

    /* Before starting to run the program, establish a stack */
    setreg(FP, memsize ? (memsize-1) : 0); /* initialize frame pointer */
    setreg(SP, memsize); /* initialize stack pointer */
    addmem(64); /* reserve memory for the stack at end of address space */

    /* Each iteration of this loop executes one instruction */
    while(!halted)
    {
        /* Fetch the instruction word */
        ir = getmem(pc);
        if(verbose)
        {
            printf("Executing ");
            disasm(mem, pc, 1);
        }
        pc++;

        /* Decode the instruction word */
        insn = decode(ir);
        reg = insn->reg;

        tmp = (ssize_t)(int16_t)insn->imm;
        if(insn->idxreg) tmp += (ssize_t)getreg(insn->idxreg);
        tr = (size_t)tmp;

        switch(insn->mode)
        {
        case 0: break;
        case 1: tr = getmem(tr); break;
        case 2: tr = getmem(getmem(tr)); break;
        }

        /* Execute the instruction */
        switch(insn->opcode)
        {
        case 0x00: /*NOP*/
            break;
        case 0x01: /*STORE*/
            setmem(tr, getreg(reg)); 
            break;
        case 0x02: /*LOAD*/
            setreg(reg, tr);
            break;
        case 0x03: /*IN*/
            if((tr >= COUNTOF(intab)) || !intab[tr]) die("no such input device");
            setreg(reg, intab[tr]());
            break;
        case 0x04: /*OUT*/
            if((tr >= COUNTOF(outtab)) || !outtab[tr]) die("no such output device");
            outtab[tr](getreg(reg));
            break;
        case 0x11: setreg(reg, getreg(reg) + tr); break; /*ADD*/
        case 0x12: setreg(reg, getreg(reg) - tr); break; /*SUB*/
        case 0x13: setreg(reg, getreg(reg) * tr); break; /*MUL*/
        case 0x14: setreg(reg, getreg(reg) / tr); break; /*DIV*/
        case 0x15: setreg(reg, getreg(reg) % tr); break; /*MOD*/
        case 0x16: setreg(reg, getreg(reg) & tr); break; /*AND*/
        case 0x17: setreg(reg, getreg(reg) | tr); break; /*OR*/
        case 0x18: setreg(reg, getreg(reg) ^ tr); break; /*XOR*/
        case 0x19: setreg(reg, getreg(reg) << tr); break; /*SHL*/
        case 0x1A: setreg(reg, size_shr(getreg(reg), tr)); break; /*SHR*/
        case 0x1B: setreg(reg, size_sar(getreg(reg), tr)); break; /*SHRA*/
        case 0x1F: compare(getreg(reg), tr); break; /*COMP*/
        case 0x20: pc=tr; break; /*JUMP*/
        case 0x21: if((ssize_t)getreg(reg) < 0) pc=tr; break; /*JNEG*/
        case 0x22: if((ssize_t)getreg(reg) == 0) pc=tr; break; /*JZER*/
        case 0x23: if((ssize_t)getreg(reg) > 0) pc=tr; break; /*JPOS*/
        case 0x24: if((ssize_t)getreg(reg) >= 0) pc=tr; break; /*JNNEG*/
        case 0x25: if((ssize_t)getreg(reg) != 0) pc=tr; break; /*JNZER*/
        case 0x26: if((ssize_t)getreg(reg) <= 0) pc=tr; break; /*JNPOS*/
        case 0x27: if(getsrbit(SR_L)) pc=tr; break; /*JLES*/
        case 0x28: if(getsrbit(SR_E)) pc=tr; break; /*JEQU*/
        case 0x29: if(getsrbit(SR_G)) pc=tr; break; /*JGRE*/
        case 0x2A: if(!getsrbit(SR_L)) pc=tr; break; /*JNLES*/
        case 0x2B: if(!getsrbit(SR_E)) pc=tr; break; /*JNEQU*/
        case 0x2C: if(!getsrbit(SR_G)) pc=tr; break; /*JNGRE*/
        case 0x31: /*CALL*/ 
            push(reg, pc);
            push(reg, getreg(FP));
            setreg(FP, getreg(SP));
            pc=tr;
            break;
        case 0x32: /*EXIT*/
            setreg(FP, pop(reg));
            pc = pop(reg);
            for(; tr; tr--) pop(reg);
            break;
        case 0x33: /*PUSH*/
            push(reg, tr);
            break;
        case 0x34: /*POP*/
            setreg(insn->idxreg, pop(reg));
            break;
        case 0x35: /*PUSHR*/
            push(reg, getreg(0));
            push(reg, getreg(1));
            push(reg, getreg(2));
            push(reg, getreg(3));
            push(reg, getreg(4));
            push(reg, getreg(5));
            break;
        case 0x36: /*POPR*/
            setreg(5, pop(reg));
            setreg(4, pop(reg));
            setreg(3, pop(reg));
            setreg(2, pop(reg));
            setreg(1, pop(reg));
            setreg(0, pop(reg));
            break;
        case 0x70: /*SVC*/
            if((tr >= COUNTOF(svctab)) || !svctab[tr]) die("no such supervisor call");
            svctab[tr](reg);
            break;
        default:
            die("bad instruction");
        }
    }
}

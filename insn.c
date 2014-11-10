/* Instruction table and instruction decoding. The table maps
 * instruction opcodes to mnemonics. The decoder is used by both the
 * disassembler and the simulator to split an instruction word into
 * its component parts as explained in the Titokone manual and the
 * slides of the University of Helsinki "Tietokoneen toiminta"
 * course. */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "die.h"
#include "insn.h"

/* Represents one entry in the instruction table */
struct insndef
{
    size_t opcode;
    char *mnemonic;
};

/* The instruction table, sorted by opcode. */
static struct insndef defs[] =
{
    {0x00, "NOP"},
    {0x01, "STORE"},
    {0x02, "LOAD"},
    {0x03, "IN"},
    {0x04, "OUT"},
    {0x11, "ADD"},
    {0x12, "SUB"},
    {0x13, "MUL"},
    {0x14, "DIV"},
    {0x15, "MOD"},
    {0x16, "AND"},
    {0x17, "OR"},
    {0x18, "XOR"},
    {0x19, "SHL"},
    {0x1A, "SHR"},
    {0x1B, "SHRA"},
    {0x1F, "COMP"},
    {0x20, "JUMP"},
    {0x21, "JNEG"},
    {0x22, "JZER"},
    {0x23, "JPOS"},
    {0x24, "JNNEG"},
    {0x25, "JNZER"},
    {0x26, "JNPOS"},
    {0x27, "JLES"},
    {0x28, "JEQU"},
    {0x29, "JGRE"},
    {0x2A, "JNLES"},
    {0x2B, "JNEQU"},
    {0x2C, "JNGRE"},
    {0x31, "CALL"},
    {0x32, "EXIT"},
    {0x33, "PUSH"},
    {0x34, "POP"},
    {0x35, "PUSHR"},
    {0x36, "POPR"},
    {0x70, "SVC"},
};

/* Count of entries in the instruction table */
static const size_t ndefs = sizeof(defs) / sizeof(defs[0]);

/* opcodemnemonic -- return an instruction's mnemonic given its opcode.
 *
 * opcode -- the opcode as disassembled from the instruction word
 * return value -- a string constant containing the mnemonic, or "" if
 * the opcode was unknown or invalid.
 */
static char *opcodemnemonic(size_t opcode)
{
    struct insndef *def;

    for(def=defs; def<defs+ndefs; def++)
        if(def->opcode == opcode)
            return(def->mnemonic);
    return("");
}

/*
 * decode -- decode an instruction word
 *
 * word -- the instruction word to decode
 * return value -- a pointer to a statically allocated insn structure
 * corresponding to the instruction word; see comments in insn.h.
 */
struct insn *decode(size_t word)
{
    static struct insn buf; /* not re-entrant */
    struct insn *insn = &buf;

    insn->opcode   = (word >> 24);
    insn->reg      = (word >> 21) & 7;
    insn->mode     = (word >> 19) & 3;
    insn->idxreg   = (word >> 16) & 7;
    insn->imm      = (ssize_t)(int16_t)(word & 0xffff);
    insn->mnemonic = opcodemnemonic(insn->opcode);
    return(insn);
}

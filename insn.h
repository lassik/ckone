/* Instruction table and instruction decoding. The table maps
 * instruction opcodes to mnemonics. The decoder is used by both the
 * disassembler and the simulator to split an instruction word into
 * its component parts as explained in the Titokone manual and the
 * slides of the University of Helsinki "Tietokoneen toiminta"
 * course. */

/* The decoded form of a single instruction */
struct insn
{
    size_t opcode;
    size_t reg; /* main register (Ri) */
    /* Operand: */
    size_t mode; /* addressing mode: 0=immediate, 1=address, 2=indirect */
    size_t idxreg; /* index register (Rj) or zero if none */
    size_t imm; /* immediate value or address */
    char *mnemonic; /* mnemonic string or "" if invalid */
};

/* decode -- decode an instruction word */
struct insn *decode(size_t word);

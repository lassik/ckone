/* Disassembler. Prints disassembled TTK-91 instructions to standard
 * output. The disassembled instructions look a lot like TTK-91
 * assembler code, but since the assembly->bytecode->disassembly
 * translation loses some information, the instructions cannot be
 * presented in their original form as they appeared in the assembly
 * language source file.
 *
 * In the output, each instruction word is prefixed by its memory
 * address.
 */

#include <stdio.h>
#include <stdlib.h>

#include "insn.h"
#include "reg.h"
#include "disasm.h"

/*
 * disasm -- disassemble instructions from a memory array.
 *
 * words  -- the memory array
 * offset -- array index of first instruction
 * count  -- how many instruction words to disassemble
 */
void disasm(size_t *words, size_t offset, size_t count)
{
    struct insn *insn;

    /* Each iteration of this loop disassembles a single instruction word. */
    for(; count; count--, offset++)
    {
        /* Memory address */
        printf("% 4zd: ", offset);

        insn = decode(words[offset]);

        /* Opcode mnemonic */
        printf("%s ", insn->mnemonic);

        /* Register */
        if(insn->reg) printf("%s, ", regnames[insn->reg]);

        /* Addressing mode */
        if(insn->mode==0) 
            printf("=");
        else if(insn->mode==2)
            printf("@");

        /* Operand */
        if(insn->imm && insn->idxreg)
            printf("%zd(%s)", insn->imm, regnames[insn->idxreg]);
        else if(insn->imm) 
            printf("%zd", insn->imm);
        else if(insn->idxreg)
            printf("%s", regnames[insn->idxreg]);
        else
            printf("0");

        printf("\n");
    }
    fflush(stdout);
}

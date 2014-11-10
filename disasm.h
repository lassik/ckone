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

/* disasm -- disassemble instructions from a memory array. */
void disasm(size_t *words, size_t offset, size_t count);

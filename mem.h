/* Represents the simulated computer's memory. */

/* An array of words containing the entire memory, expandable at will. */ 
extern size_t *mem;

/* Current size in words of the entire memory. */
extern size_t memsize;

/* Offset in memory of first code word */
extern size_t codeoff;

/* Size in words of code area */
extern size_t codesize;

/* Offset in memory of first data word */
extern size_t dataoff;

/* Size in words of data area */
extern size_t datasize;

void addmem(size_t increment);
void setmem(size_t addr, size_t word);
size_t getmem(size_t addr);

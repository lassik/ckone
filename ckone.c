/* ckone main program. Parses command line arguments, then calls the
 * other modules of the program in the right sequence. Some modules
 * are only called in verbose mode. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "die.h"
#include "parser.h"
#include "mem.h"
#include "sym.h"
#include "disasm.h"
#include "sim.h"

/*
 * Values from command line arguments
 */

/* Whether to generate non-essential output. This is false (zero)
 * initially, but the -v command line option sets it to true
 * (nonzero). */
int verbose;

/* The file name of the .b91 input file */
static char *file;

/*
 * usage -- print instructions on command line usage and exit. Called
 * if the command line syntax is incorrect or there are unknown
 * command line options.
 */
static void usage(void)
{
    fprintf(stderr, "usage: ckone [-v] file.b91\n");
    exit(1);
}

/*
 * main -- standard C main program.
 */
int main(int argc, char **argv)
{
    int i;

    /* Parse command line arguments */
    i = 1;
    while((i<argc) && (argv[i][0]=='-'))
    {
        if(!strcmp(argv[i], "--")) { i++; break; }
        else if(!strcmp(argv[i], "-v")) verbose = 1;
        else if(!strcmp(argv[i], "-h")) usage();
        else usage();
        i++;
    }
    if(i != argc-1) usage();
    file = argv[i];

    /* Engage the simulator! */
    parsefile(file);
    if(verbose)
    {
        printf("Disassembly of code area at program start:\n");
        disasm(mem, codeoff, codesize);
        printf("\n");
        printf("Running program:\n");
    }
    simulate();
    if(verbose)
    {
        printf("\n");
        printf("Data area symbols at program halt:\n");
        printsymtab();
    }
    return(0);
}

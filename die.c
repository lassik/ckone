/* Routines for doing error-exits */

#include <stdio.h>
#include <stdlib.h>

#include "die.h"

/*
 * die -- error-exit with the given message
 *
 * msg -- the message
 */
void die(char *msg)
{
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

/*
 * die -- error-exit with the given message
 *
 * msg -- the message
 * s -- a string to append to the message
 */
void dies(char *msg, char *s)
{
    fprintf(stderr, "error: %s %s\n", msg, s);
    exit(1);
}

/* Routines for doing error-exits */

/* Macro to append to the declaration of any function F to tell the
 * compiler that F does not return, thereby suppressing spurious
 * warnings about missing return statements in functions that call
 * F. This is non-standard so it must be defined separately for each
 * compiler. On the other hand, leaving the definition blank does no
 * harm besides producing the spurious warnings. */
#ifndef NORETURN
# ifdef __GCC__
#  define NORETURN __attribute__((noreturn))
# else
#  define NORETURN /* intentionally blank */
# endif
#endif

void die(char *msg) NORETURN;
void dies(char *msg, char *s) NORETURN;

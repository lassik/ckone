/* Parser for the .b91 file format that describes pre-assembled TTK-91
 * programs. */

#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "size.h"
#include "die.h"
#include "sym.h"
#include "mem.h"
#include "parser.h"

/*
 * Parser state
 */
                     
/* Maximum length of a token in characters, not counting the null terminator. */
#define MAXTOKLEN 255

/* Input stream, doesn't matter whether it is in text or binary mode. */
static FILE *input;

/* Buffer for the next input token. Null-terminated. */
static char buf[MAXTOKLEN+1];

/*
 * Character reading
 */

/*
 * readchar -- read the next character (really a byte) from the input stream
 *
 * return value -- the character, either character value 0..255 or EOF
 *
 * Dies on read error. Note that since we use null-terminated strings
 * to store tokens, we must not allow null bytes as part of the input.
 */
static int readchar(void)
{
    int ch;

    ch = fgetc(input);
    if(ferror(input)) die("cannot read from input file");
    if(!ch) die("null byte in input");
    return(ch);
}

/*
 * maybereadthechar -- read the next character from the input stream if it equals goal
 *
 * goal -- the goal character to be tested against
 * return value -- if the next character equaled goal, that character. Otherwise zero.
 */
static int maybereadthechar(int goal)
{
    int ch;

    ch = readchar();
    if(ch == goal) return(ch);
    ungetc(ch, input);
    return(0);
}

/*
 * maybereadcharin -- read the next character from the input stream if it appears in goal
 *
 * goal -- the character class (a string of the characters that are
 * part of the class) to be tested against
 * return value -- if the next character was part of the class, that character. Otherwise zero.
 */
static int maybereadcharin(const char *goal)
{
    int ch;

    ch = readchar();
    if(strchr(goal, ch)) return(ch);
    ungetc(ch, input);
    return(0);
}

/*
 * skip -- skip all following characters that appear in goal
 *
 * goal -- the character class to be tested against
 *
 * This function is used to skip whitespace or blank lines between
 * tokens.
 */
static void skip(const char *goal)
{
    while(maybereadcharin(goal));
}

/*
 * readintobuf -- read a token into the static token buffer buf
 *
 * firstch -- first character of the token, assumed to already have
 * been read by the caller
 * tailgoal -- character class into which characters comprising the
 * rest of the token must belong
 *
 * Reads the next token, one or more characters long, into the static
 * token buffer buf. buf is guaranteed to be null-terminated upon exit
 * from this function. buf is of fixed size; this function dies if the
 * size is exceeded without seeing the end of the token. Truncated
 * tokens are never returned.
 */
static void readintobuf(int firstch, const char *tailgoal)
{
    size_t n;
    int ch;

    n = 0;
    ch = firstch;
    for(;;)
    {
        if(n >= MAXTOKLEN) die("input token too long");
        buf[n++] = ch;
        ch = maybereadcharin(tailgoal);
        if(!ch) break;
    }
    buf[n] = 0;
}

/*
 * Tokenizer
 */

/* Preprocessor macros to use as building blocks in character
 * classes. C handily allows us to concatenate constant strings to
 * form a longer string, so by using preprocessor macros we can simply
 * concatenate the names of these "subclasses" to form character
 * classes. */
#define UPPERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWERS "abcdefghijklmnopqrstuvwxyz"
#define DIGITS "0123456789"
#define NEWLNS "\n\r"
#define WHITES " \t"

static const char *eols          = NEWLNS;
static const char *blanklines    = WHITES NEWLNS;
static const char *whitespace    = WHITES;
static const char *bodychars     = UPPERS LOWERS DIGITS "_";
static const char *symstartchars = UPPERS LOWERS;
static const char *digits        = DIGITS;

/*
 * The following reading functions each read a single token of the
 * indicated type from the input stream. All except those with "maybe"
 * in their names die if the next token is not of the desired type.
 */

/*
 * readeof -- read end of file or die
 */
static void readeof(void)
{
    skip(blanklines);
    if(!maybereadthechar(EOF)) die("end of file expected");
}

/*
 * readeol -- read end of line or die
 *
 * Note all following blank lines are consumed.
 */
static void readeol(void)
{
    skip(whitespace);
    if(!maybereadthechar(EOF) && !maybereadcharin(eols)) die("end of line expected");
    skip(blanklines);
}

/*
 * readkeyword -- read a line containing the given keyword or die
 *
 * keyword -- the keyword that must appear next in the input stream
 *
 * Note all following blank lines are consumed.
 */
static void readkeyword(char *keyword)
{
    skip(blanklines);
    if(!maybereadthechar('_')) die("keyword expected");
    readintobuf('_', bodychars);
    if(strcmp(buf, keyword)) die("different keyword expected");
    skip(blanklines);
}

/*
 * maybereadsym -- read a symbol or return zero
 *
 * return value -- If the next token in the stream is a symbol, a
 * pointer to the static token buffer buf, which is guaranteed to
 * contain a null-terminated string. Otherwise a null pointer.
 */
static char *maybereadsym(void)
{
    int ch;

    skip(whitespace);
    if(!(ch = maybereadcharin(symstartchars))) return(0);
    readintobuf(ch, bodychars);
    return(buf);
}

/*
 * readsize -- read a signed decimal integer from the input stream,
 * returning it as an unsigned size_t
 *
 * min -- the minimum allowed value of the unsigned result (0 or greater)
 * max -- the maximum allowed value of the unsigned result (SIZE_MAX or less)
 * return value -- the unsigned result
 *
 * The unsigned result is the two's complement representation of the
 * signed input integer. The signed version may later be recovered by
 * casting the result into (ssize_t) which is a signed integer with
 * the same number of bits as size_t. This assumes the underlying C
 * implementation uses two's complement.
 */
static size_t readsize(size_t min, size_t max)
{
    size_t val;
    char *digit;
    int ch;
    int sign;

    skip(whitespace);
    val = 0;
    digit = 0;
    sign = maybereadthechar('-') ? 1 : 0; /* negative number? */
    while((ch = maybereadcharin(digits)))
    {
        if(!(digit = strchr(digits, ch))) die("syntax error");
        val = size_mul(val, 10);
        val = size_add(val, digit-digits);
    }
    if(!digit) die("integer expected"); /* did we actually read any digits? if not, die. */
    if(sign)
    {
        /* The upper bound here comes from the definition of the two's
         * complement system of integer representation. For example,
         * if SIZE_BIT is 32 meaning size_t is 32 bits wide, then
         * 1<<(SIZE_BIT-1) == 1<<31, which is the exclusive upper
         * bound for _nonnegative_ 32-bit two's complement
         * integers. We use this bound rather than the one for
         * _negative_ integers because we first take the absolute
         * value of the negative number and then negate it to get the
         * actual number we want; the absolute value must be
         * representable too. */
        if(val >= ((size_t)1<<(SIZE_BIT-1))) die("magnitude too large for negative integer");
        val = (size_t)(-(ssize_t)val);
    }
    if(val < min) die("integer too small");
    if(val > max) die("integer too large");
    return(val);
}

/*
 * Parser
 */

/*
 * readdump -- read a dump of an area (code or data area) of the simulated computer's memory
 *
 * out_off -- pointer to output parameter into which the section's memory offset is stored
 * out_size -- pointer to output parameter into which the section's size in memory words is stored
 *
 * The section has (last-off)+1 words and in the file it is of the form
 *
 * <off> <last>
 * <word0>
 * <word1>
 * <word2>
 * ...
 * <wordn>
 */
static void readdump(size_t *out_off, size_t *out_size)
{
    size_t off, size, last, i;

    off = readsize(memsize, memsize);
    last = readsize(off, SIZE_MAX-1);
    readeol();

    size = (last-off)+1;
    addmem(size);
    for(i=off; i<=last; i++)
    {
        mem[i] = readsize(0, SIZE_MAX);
        readeol();
    }

    *out_off = off;
    *out_size = size;
}

/*
 * readsymtab -- read the symbol table from the input stream
 *
 * The symbol table is of the form
 *
 * <symbol1> <offset1>
 * <symbol2> <offset2>
 * ...
 * <symboln> <offsetn>
 *
 * The table is terminated by the keyword introducing the end of the
 * file. This is a bad design decision in the file format because we
 * have to use conditional reading on the symbols; it would have been
 * better to state the count of symbols in the file before the symbols
 * themselves.
 */
static void readsymtab(void)
{
    char *sym;

    while((sym = maybereadsym()))
    {
        addsym(sym, readsize(0, SIZE_MAX));
        readeol();
    }
}

/*
 * readinput -- read a .b91 file from the input stream, storing the
 * results in the global memory array.
 */
static void readinput(void)
{
    readkeyword("___b91___");
    readkeyword("___code___");
    readdump(&codeoff, &codesize);
    readkeyword("___data___");
    readdump(&dataoff, &datasize);
    readkeyword("___symboltable___");
    readsymtab();
    readkeyword("___end___");
    readeof();
}

/*
 * parsefile -- read a .b91 file from the named file into the global
 * memory array
 *
 * filename -- the name of the file
 */
void parsefile(char *filename)
{
    if(!(input = fopen(filename, "rb"))) die("cannot open input file");
    readinput();
    if(fclose(input)) die("cannot close input file");
}

/* Safe and portable operations on size_t values */

#include <limits.h>
#include <stdlib.h>

#include "size.h"
#include "die.h"

/*
 * size_add -- add two size_t values, die on overflow
 *
 * a and b -- operands
 * return value -- sum
 */
size_t size_add(size_t a, size_t b)
{
    if(SIZE_MAX-a < b) die("integer overflow");
    return(a+b);
}

/*
 * size_mul -- multiply two size_t values, die on overflow
 *
 * x and n -- operands
 * return value -- product
 *
 * Note that for "speed" the smaller value should be n, not x.
 */
size_t size_mul(size_t x, size_t n)
{
    size_t c = 0;
    for(; n; n--) c = size_add(c, x);
    return(c);
}

/*
 * size_shr -- _logical_ right-shift of a size_t value
 *
 * val -- the value to be shifted
 * nbits -- how many bits to shift by, modulo SIZE_BIT
 * return value -- the result
 */
size_t size_shr(size_t val, size_t nbits)
{
    return((val>>nbits) & (SIZE_MAX>>(nbits&(SIZE_BIT-1))));
}

/*
 * size_sar -- _arithmetic_ right-shift of a size_t value
 *
 * val -- the value to be shifted
 * nbits -- how many bits to shift by, modulo SIZE_BIT
 * return value -- the result
 */
size_t size_sar(size_t val, size_t nbits)
{
    if(!(val&((size_t)1<<(SIZE_BIT-1)))) return(size_shr(val, nbits));
    return((val>>nbits) | ~(SIZE_MAX>>(nbits&(SIZE_BIT-1))));
}

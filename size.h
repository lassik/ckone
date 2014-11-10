/* Safe and portable operations on size_t values */

/* How many bits wide the size_t (and ssize_t) type is. */
#ifndef SIZE_BIT
#define SIZE_BIT (CHAR_BIT * sizeof(size_t))
#endif

/* Maximum value of the unsigned size_t value. */
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

size_t size_add(size_t a, size_t b);
size_t size_mul(size_t x, size_t n);
size_t size_shr(size_t val, size_t nbits);
size_t size_sar(size_t val, size_t nbits);

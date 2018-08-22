#ifndef BITOPS_DEF_H_
#define BITOPS_DEF_H_

#define BIT1(i) (1<<(i))
#define BIT(i, b) (((b) ? (BIT1(i)) : 0))
#define BIT_READ(b, mask)  ((b) &   (mask))
#define BIT_SET1(b, mask)  ((b) |=  (mask))
#define BIT_CLEAR(b, mask) ((b) &= ~(mask))
#define BIT_XOR(b, mask)   ((b) ^=  (mask))

#endif /* BITOPS_DEF_H_ */

#ifndef PLINC_FPMATH_H
#define PLINC_FPMATH_H

#define PLINC_BEGINFP()     /**/
#define PLINC_ENDFP(x)      (isnan(x) || isinf(x))

double  PlincToRadians(float f);

#endif /* PLINC_FPMATH_H */

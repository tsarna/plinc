#ifndef PLINC_MATRIX_H
#define PLINC_MATRIX_H

#include <plinc/types.h>

typedef struct _PlincMatrix {
    PlincReal   A, B, C, D, Tx, Ty;
} PlincMatrix;

#define PLINC_MATRIX_TYPE       (PLINC_ATTR_LIT | PLINC_TYPE_ARRAY | 6)

#define PLINC_IS_MATRIX(v)      ( \
 ((v).Flags & (PLINC_TYPE_MASK|PLINC_SIZE_MASK|PLINC_ATTR_NOWRITE)) \
 == (PLINC_TYPE_ARRAY|6) \
)
        
extern const PlincMatrix PlincIdentityMatrix;

#ifdef WITH_SIMPLE_CTM
#define PLINC_CTM(i)                ((i)->CTM)
#define PLINC_DEFAULT_MATRIX(i)     ((i)->DefaultMatrix)

extern const PlincMatrix PlincDefaultMatrix;
#endif

void     PlincConcatMatrix(PlincMatrix *r, PlincMatrix *a, PlincMatrix *b);
void     PlincTransformByMatrix(PlincMatrix *r, PlincReal *x, PlincReal *y);
void     PlincDTransformByMatrix(PlincMatrix *r, PlincReal *x, PlincReal *y);
#if 0
void    *PlincSetMatrixVal(PlincInterp *i, PlincVal *v, const PlincMatrix *m);
void    *PlincGetMatrixVal(PlincInterp *i, PlincVal *v, PlincMatrix *m);
#endif


#endif /* PLINC_MATRIX_H */

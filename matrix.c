#ifdef WITH_MATRIX

#include <plinc/interp.h>
#include <plinc/matrix.h>
#include <plinc/fpmath.h>

#include <stdlib.h>
#include <math.h>


const PlincMatrix PlincIdentityMatrix = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
#ifdef WITH_DEFAULT_DEFAULT_MATRIX
const PlincMatrix PlincDefaultMatrix = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
#endif



void
PlincConcatMatrix(PlincMatrix *r, PlincMatrix *a, PlincMatrix *b)
{
    r->A  = ((a->A)  * (b->A) + (a->B)  * (b->C));
    r->B  = ((a->A)  * (b->B) + (a->B)  * (b->D));
    r->C  = ((a->C)  * (b->A) + (a->D)  * (b->C));
    r->D  = ((a->C)  * (b->B) + (a->D)  * (b->D));
    r->Tx = ((a->Tx) * (b->A) + (a->Ty) * (b->C) + (b->Tx));
    r->Ty = ((a->Tx) * (b->B) + (a->Ty) * (b->D) + (b->Ty));
}



void
PlincTransformByMatrix(PlincMatrix *m, PlincReal *x, PlincReal *y)
{
    float tx;
    
    tx = (m->A)*(*x) + (m->C)*(*y) + (m->Tx);
    *y = (m->B)*(*x) + (m->D)*(*y) + (m->Ty);
    *x = tx;
}



void
PlincDTransformByMatrix(PlincMatrix *m, PlincReal *x, PlincReal *y)
{
    float tx;
    
    tx = (m->A)*(*x) + (m->C)*(*y);
    *y = (m->B)*(*x) + (m->D)*(*y);
    *x = tx;
}



void *
PlincSetMatrixVal(PlincInterp *i, PlincVal *v, const PlincMatrix *m)
{
    PlincVal nv;
    void *r;
    
    if (!PLINC_IS_MATRIX(*v)) {
        return i->typecheck;
    } else {
        nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_REAL;
        
        nv.Val.Real = m->A;
        r = PlincPutArray(i, v->Val.Ptr, 0, &nv);
        if (!r) {
            nv.Val.Real = m->B;
            r = PlincPutArray(i, v->Val.Ptr, 1, &nv);
        }
        if (!r) {
            nv.Val.Real = m->C;
            r = PlincPutArray(i, v->Val.Ptr, 2, &nv);
        }
        if (!r) {
            nv.Val.Real = m->D;
            r = PlincPutArray(i, v->Val.Ptr, 3, &nv);
        }
        if (!r) {
            nv.Val.Real = m->Tx;
            r = PlincPutArray(i, v->Val.Ptr, 4, &nv);
        }
        if (!r) {
            nv.Val.Real = m->Ty;
            r = PlincPutArray(i, v->Val.Ptr, 5, &nv);
        }
    }
    
    return r;
}



void *
PlincGetMatrixVal(PlincInterp *i, PlincVal *v, PlincMatrix *m)
{
    if (!PLINC_IS_MATRIX(*v)) {
        return i->typecheck;
    } else {
        v = (PlincVal *)(v->Val.Ptr);
        
#define OK(n)   PLINC_IS_NUM(v[n])

        if (!(OK(0) && OK(1) && OK(2) && OK(3) && OK(4) && OK(5))) {
            return i->typecheck;
        } else {
            m->A  = PLINC_NUM_VAL(v[0]);
            m->B  = PLINC_NUM_VAL(v[1]);
            m->C  = PLINC_NUM_VAL(v[2]);
            m->D  = PLINC_NUM_VAL(v[3]);
            m->Tx = PLINC_NUM_VAL(v[4]);
            m->Ty = PLINC_NUM_VAL(v[5]);

            return NULL;
        }
    }
}



static void *
op_matrix(PlincInterp *i)
{
    PlincVal v;
    void *r;

    
    if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        v.Flags = PLINC_MATRIX_TYPE;
        v.Val.Ptr = PlincNewArray(i->Heap, 6);
        if (!v.Val.Ptr) {
            return i->VMerror;
        }
        
        r = PlincSetMatrixVal(i, &v, &PlincIdentityMatrix);
	if (!r) {
            PLINC_OPPUSH(i, v);
        }

        return r;
    }
}



static void *
op_initmatrix(PlincInterp *i)
{
    PLINC_CTM(i) = PLINC_DEFAULT_MATRIX(i);

    return NULL;
}



static void *
op_identmatrix(PlincInterp *i)
{
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        return PlincSetMatrixVal(i, &PLINC_OPTOPDOWN(i, 0), &PlincIdentityMatrix);
    }
}



static void *
op_defaultmatrix(PlincInterp *i)
{
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        return PlincSetMatrixVal(i, &PLINC_OPTOPDOWN(i, 0),
            &PLINC_DEFAULT_MATRIX(i));
    }
}



static void *
op_currentmatrix(PlincInterp *i)
{
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        return PlincSetMatrixVal(i, &PLINC_OPTOPDOWN(i, 0), &PLINC_CTM(i));
    }
}



static void *
op_setmatrix(PlincInterp *i)
{
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        r = PlincGetMatrixVal(i, &PLINC_OPTOPDOWN(i, 0), &PLINC_CTM(i));
        if (!r) {
            PLINC_OPPOP(i);
        }
        
        return r;
    }
}



static void *
op_translate(PlincInterp *i)
{
    PlincMatrix T, rm;
    PlincVal *v, *x, *y, rv;
    void *r = NULL;
    
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        if (PLINC_IS_MATRIX(*v)) {
            if (!PLINC_OPSTACKHAS(i, 3)) {
                return i->stackunderflow;
            } else {
                x = &PLINC_OPTOPDOWN(i, 2);
                y = &PLINC_OPTOPDOWN(i, 1);
            }
        } else {
            x = &PLINC_OPTOPDOWN(i, 1);
            y = &PLINC_OPTOPDOWN(i, 0);
        }

        if (!PLINC_IS_NUM(*x) || !PLINC_IS_NUM(*y)) {
            return i->typecheck;
        } else {
            T.A  = T.D = 1.0;
            T.B  = T.C = 0.0;
            T.Tx = PLINC_NUM_VAL(*x);
            T.Ty = PLINC_NUM_VAL(*y);

            if (PLINC_IS_MATRIX(*v)) {
                rv = *v;
                r = PlincSetMatrixVal(i, &rv, &T);
                if (!r) {
                    PLINC_OPPOP(i);
                    PLINC_OPPOP(i);
                    PLINC_OPPOP(i);
                    PLINC_OPPUSH(i, rv);
                }
            } else {
                PlincConcatMatrix(&rm, &T, &PLINC_CTM(i));
                PLINC_CTM(i) = rm;
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
            }

        }
        
        return r;
    }
}



static void *
op_scale(PlincInterp *i)
{
    PlincMatrix T, rm;
    PlincVal *v, *x, *y, rv;
    void *r = NULL;
    
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        if (PLINC_IS_MATRIX(*v)) {
            if (!PLINC_OPSTACKHAS(i, 3)) {
                return i->stackunderflow;
            } else {
                x = &PLINC_OPTOPDOWN(i, 2);
                y = &PLINC_OPTOPDOWN(i, 1);
            }
        } else {
            x = &PLINC_OPTOPDOWN(i, 1);
            y = &PLINC_OPTOPDOWN(i, 0);
        }

        if (!PLINC_IS_NUM(*x) || !PLINC_IS_NUM(*y)) {
            return i->typecheck;
        } else {
            T.B = T.C = T.Tx = T.Ty = 0.0;
            T.A = PLINC_NUM_VAL(*x);
            T.D = PLINC_NUM_VAL(*y);

            if (PLINC_IS_MATRIX(*v)) {
                rv = *v;
                r = PlincSetMatrixVal(i, &rv, &T);
                if (!r) {
                    PLINC_OPPOP(i);
                    PLINC_OPPOP(i);
                    PLINC_OPPOP(i);
                    PLINC_OPPUSH(i, rv);
                }
            } else {
                PlincConcatMatrix(&rm, &T, &PLINC_CTM(i));
                PLINC_CTM(i) = rm;
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
            }

        }
        
        return r;
    }
}



static void *
op_rotate(PlincInterp *i)
{
    PlincMatrix T, rm;
    PlincVal *v, *a, rv;
    double rad;
    void *r = NULL;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        if (PLINC_IS_MATRIX(*v)) {
            if (!PLINC_OPSTACKHAS(i, 2)) {
                return i->stackunderflow;
            } else {
                a = &PLINC_OPTOPDOWN(i, 1);
            }
        } else {
            a = &PLINC_OPTOPDOWN(i, 0);
        }

        if (!PLINC_IS_NUM(*a)) {
            return i->typecheck;
        } else {
            rad = PlincToRadians(PLINC_NUM_VAL(*a));
            T.A = T.D = cos(rad);
            T.B = sin(rad);
            T.C = -T.B;
            T.Tx = T.Ty = 0.0;

            if (PLINC_IS_MATRIX(*v)) {
                rv = *v;
                r = PlincSetMatrixVal(i, &rv, &T);
                if (!r) {
                    PLINC_OPPOP(i);
                    PLINC_OPPOP(i);
                    PLINC_OPPUSH(i, rv);
                }
            } else {
                PlincConcatMatrix(&rm, &T, &PLINC_CTM(i));
                PLINC_CTM(i) = rm;
                PLINC_OPPOP(i);
            }

        }
        
        return r;
    }
}



static void *
op_concat(PlincInterp *i)
{
    PlincMatrix rm, ra;
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        r = PlincGetMatrixVal(i, &PLINC_OPTOPDOWN(i, 0), &ra);
        if (!r) {
            PlincConcatMatrix(&rm, &ra, &PLINC_CTM(i));
            PLINC_CTM(i) = rm;
            PLINC_OPPOP(i);
        }
        
        return r;
    }
}



static void *
op_concatmatrix(PlincInterp *i)
{
    PlincVal *mv1, *mv2, *mv3, mrv;
    PlincMatrix m1, m2, m3;
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 3)) {
        return i->stackunderflow;
    } else {
        mv1 = &PLINC_OPTOPDOWN(i, 2);
        mv2 = &PLINC_OPTOPDOWN(i, 1);
        mv3 = &PLINC_OPTOPDOWN(i, 0);
        if (!PLINC_IS_MATRIX(*mv3)) {
            return i->typecheck;
        } else {
            r = PlincGetMatrixVal(i, mv1, &m1);
            if (!r) {
                r = PlincGetMatrixVal(i, mv2, &m2);
            }
            if (!r) {
                PlincConcatMatrix(&m3, &m1, &m2);
                r = PlincSetMatrixVal(i, mv3, &m3);
            }
            if (!r) {
                mrv = *mv3;
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, mrv);
            }
        }
        
        return r;
    }
}



static void *
do_transform(PlincInterp *i, int distance)
{
    PlincMatrix m, *tm = &m;
    PlincVal *v, *vx, *vy;
    PlincReal x, y;
    int threearg = FALSE;
    void *r = NULL;
    
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        if (PLINC_IS_MATRIX(*v)) {
            threearg = TRUE;
            if (!PLINC_OPSTACKHAS(i, 3)) {
                return i->stackunderflow;
            } else {
                vx = &PLINC_OPTOPDOWN(i, 2);
                vy = &PLINC_OPTOPDOWN(i, 1);
            }
        } else {
            vx = &PLINC_OPTOPDOWN(i, 1);
            vy = &PLINC_OPTOPDOWN(i, 0);
        }

        if (!PLINC_IS_NUM(*vx) || !PLINC_IS_NUM(*vy)) {
            return i->typecheck;
        } else {
            x = PLINC_NUM_VAL(*vx);
            y = PLINC_NUM_VAL(*vy);
            
            if (threearg) {
                r = PlincGetMatrixVal(i, v, &m);
                tm = &m;
                if (r) {
                    return r;
                }
            } else {
                tm = &PLINC_CTM(i);
            }
            
            if (distance) {
                PlincDTransformByMatrix(tm, &x, &y);
            } else {
                PlincTransformByMatrix(tm, &x, &y);
            }

            vx->Flags = vy->Flags = PLINC_ATTR_LIT | PLINC_TYPE_REAL;
            vx->Val.Real = x;
            vy->Val.Real = y;
            
            if (threearg) {
                PLINC_OPPOP(i);
            }
        }
        
        return r;
    }
}



static void *
op_transform(PlincInterp *i)
{
    return do_transform(i, FALSE);
}



static void *
op_dtransform(PlincInterp *i)
{
    return do_transform(i, TRUE);
}



static const PlincOp ops[] = {
    {op_matrix,         "matrix"},
    {op_initmatrix,     "initmatrix"},
    {op_identmatrix,    "identmatrix"},
    {op_defaultmatrix,  "defaultmatrix"},
    {op_currentmatrix,  "currentmatrix"},
    {op_setmatrix,      "setmatrix"},
    {op_translate,      "translate"},
    {op_scale,          "scale"},
    {op_rotate,         "rotate"},
    {op_concat,         "concat"},
    {op_concatmatrix,   "concatmatrix"},
    {op_transform,      "transform"},
    {op_dtransform,     "dtransform"},

    {NULL,              NULL}
};



void
PlincInitMatrixOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}
#endif

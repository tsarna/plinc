/* $Endicor: arith.c,v 1.5 1999/01/20 23:14:23 tsarna Exp $ */

#include <plinc/interp.h>

#include <stdlib.h>

#ifdef WITH_REAL
#include <math.h>
#endif


static void *
idiv_mod(PlincInterp *i, int op)
{
    PlincVal *v1, *v2, v;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 0);
        v2 = &PLINC_OPTOPDOWN(i, 1);

        if ((PLINC_TYPE(*v1) != PLINC_TYPE_INT)
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_INT)) {
            return i->typecheck;
        } else if (v1->Val.Int == 0) {
            return i->undefinedresult;
        } else {
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
            if (op) {
                v.Val.Int = v2->Val.Int / v1->Val.Int;
            } else {
                v.Val.Int = v2->Val.Int % v1->Val.Int;
            }

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);

            return NULL;
        }
    }
}



static void *
op_idiv(PlincInterp *i)
{
    return idiv_mod(i, TRUE);
}



static void *
op_mod(PlincInterp *i)
{
    return idiv_mod(i, FALSE);
}



static void *
addsub(PlincInterp *i, int add)
{
    PlincVal *v1, *v2, v;
    int t1, t2;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);

        t1 = PLINC_TYPE(*v1);
        t2 = PLINC_TYPE(*v2);

        if ((t1 == PLINC_TYPE_INT) && (t2 == PLINC_TYPE_INT)) {
            int x, y, a, b;
            
            a = v1->Val.Int;
            b = v2->Val.Int;
            x = a + b;
            y = a - b;
            
            if ((add && ((x^a) < 0 && (x^b) < 0))
            ||  (!add && ((y^a) < 0 && (y^~b) < 0))) {
#ifdef WITH_REAL
                /* it would overflow, so convert to reals */

                v1->Val.Real = (float)(v1->Val.Int);
                v2->Val.Real = (float)(v2->Val.Int);

                t1 = t2 = PLINC_TYPE_REAL;
#else
                return i->rangecheck;
#endif
            } else {
                v.Flags = PLINC_TYPE_INT;
                v.Val.Int = add ? x : y;

                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, v);
                
                return NULL;
            }
        }
        
#ifdef WITH_REAL
        if ((t1 == PLINC_TYPE_INT) && (t2 == PLINC_TYPE_REAL)) {
                v1->Val.Real = (float)(v1->Val.Int);

                t1 = PLINC_TYPE_REAL;
        }

        if ((t1 == PLINC_TYPE_REAL) && (t2 == PLINC_TYPE_INT)) {
                v2->Val.Real = (float)(v2->Val.Int);

                t2 = PLINC_TYPE_REAL;
        }

        if ((t1 == PLINC_TYPE_REAL) && (t2 == PLINC_TYPE_REAL)) {
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_REAL;
            if (add) {
                v.Val.Real = v1->Val.Real + v2->Val.Real;
            } else {
                v.Val.Real = v1->Val.Real - v2->Val.Real;
            }

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);
            
            return NULL;
        }
#endif

        return i->typecheck;
    }
}



static void *
op_add(PlincInterp *i)
{
    return addsub(i, TRUE);
}



static void *
op_sub(PlincInterp *i)
{
    return addsub(i, FALSE);
}



static void *
op_abs(PlincInterp *i)
{
    PlincVal *v;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v) == PLINC_TYPE_INT) {
            if (v->Val.Int < 0) {
                v->Val.Int = -(v->Val.Int);
            }
#ifdef WITH_REAL
        } else if (PLINC_TYPE(*v) == PLINC_TYPE_REAL) {
            v->Val.Real = fabsf(v->Val.Real);
#endif
        } else {
            return i->typecheck;
        }
    }

    return NULL;
}



static void *
op_neg(PlincInterp *i)
{
    PlincVal *v;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v) == PLINC_TYPE_INT) {
            v->Val.Int = -(v->Val.Int);
#ifdef WITH_REAL
        } else if (PLINC_TYPE(*v) == PLINC_TYPE_REAL) {
            v->Val.Real = -(v->Val.Real);
#endif
        } else {
            return i->typecheck;
        }
    }

    return NULL;
}



static const PlincOp ops[] = {
    {"add",         op_add},

    {"idiv",        op_idiv},
    {"mod",         op_mod},

    {"sub",         op_sub},
    {"abs",         op_abs},
    {"neg",         op_neg},

    {NULL,          NULL}
};



void
PlincInitArithOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

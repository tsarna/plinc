/* $Endicor: arith.c,v 1.3 1999/01/20 20:16:56 tsarna Exp tsarna $ */

#include <plinc/interp.h>

#include <stdlib.h>

#ifdef WITH_REAL
#include <math.h>
#endif


static void *
op_idiv(PlincInterp *i)
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
            v.Val.Int = v2->Val.Int / v1->Val.Int;

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);

            return NULL;
        }
    }
}



#ifdef notyet
static void *
op_sub(PlincInterp *i)
{
    PlincVal *v1, *v2, v;
    int t1, t2;
    float f2;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 0);
        v2 = &PLINC_OPTOPDOWN(i, 1);

        t1 = PLINC_TYPE(*v1);
        t2 = PLINC_TYPE(*v2);

        if (t1 == PLINC_TYPE_INT) {
            if (t2 == PLINC_TYPE_INT) {
            } else if (t2 == PLINC_TYPE_REAL) {
            } else {
                return i->typecheck;
            }
        } else if (t1 == PLINC_TYPE_REAL) {
            f2 = v2->Val.Real;

            if (t2 == PLINC_TYPE_INT) {
                /* cast to real */

                f2 = (float)(v2->Val.Int;
                t2 = PLINC_TYPE_REAL;
            }

            if (t2 == PLINC_TYPE_REAL) {

            } else {
                return i->typecheck;
            }
        } else {
            return i->typecheck;
        }
    }
}
#endif



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
        } if (PLINC_TYPE(*v) == PLINC_TYPE_REAL) {
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
        } if (PLINC_TYPE(*v) == PLINC_TYPE_REAL) {
            v->Val.Real = -(v->Val.Real);
#endif
        } else {
            return i->typecheck;
        }
    }

    return NULL;
}



static const PlincOp ops[] = {
    {"idiv",        op_idiv},

    {"abs",         op_abs},
    {"neg",         op_neg},

    {NULL,          NULL}
};



void
PlincInitArithOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

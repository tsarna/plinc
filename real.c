#ifdef WITH_REAL

/* $Endicor: arith.c,v 1.7 1999/01/24 03:47:42 tsarna Exp $ */

#include <plinc/interp.h>
#include <plinc/fpmath.h>

#include <stdlib.h>

#ifdef WITH_REAL
#include <math.h>
#endif


static void *
op_div(PlincInterp *i)
{
    PlincVal *v1, *v2, v;
    float a, b, c;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v1) == PLINC_TYPE_INT) {
            a = (float)(v1->Val.Int);
        } else if (PLINC_TYPE(*v1) == PLINC_TYPE_REAL) {
            a = v1->Val.Real;
        } else {
            return i->typecheck;
        }
            
        if (PLINC_TYPE(*v2) == PLINC_TYPE_INT) {
            b = (float)(v2->Val.Int);
        } else if (PLINC_TYPE(*v2) == PLINC_TYPE_REAL) {
            b = v2->Val.Real;
        } else {
            return i->typecheck;
        }
        
        PLINC_BEGINFP();
        
        c = a / b;
        
        if (PLINC_ENDFP(c)) {
            return i->undefinedresult;
        } else {
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_REAL;
            v.Val.Real = c;            

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);

            return NULL;
        }
    }
}



static const PlincOp ops[] = {
    {op_div,        "div"},

    {NULL,          NULL}
};



void
PlincInitRealOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

#endif /* WITH_REAL */

/* $Endicor: arith.c,v 1.2 1999/01/18 00:54:54 tsarna Exp $ */

#include <plinc/interp.h>

#include <stdlib.h>



static void *
op_put(PlincInterp *i)
{
    PlincVal *v0, *v1, *v2;
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 3)) {
        return i->stackunderflow;
    } else {
        v0 = &PLINC_OPTOPDOWN(i, 0);
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 2);
        r = i->typecheck;

        if (PLINC_TYPE(*v2) == PLINC_TYPE_DICT) {
            r = PlincPutDict(i, (PlincDict *)(v2->Val.Ptr), v1, v0);
        }

        if (!r) {
            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
        }
        
        return r;
    }
}



static PlincOp ops[] = {
    {"put",         op_put},

    {NULL,          NULL}
};



void
PlincInitPolymorphOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

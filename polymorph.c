/* $Endicor: polymorph.c,v 1.2 1999/01/20 04:18:31 tsarna Exp $ */

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



static void *
op_length(PlincInterp *i)
{
    PlincVal *v, nv;
    int t;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        t = PLINC_TYPE(*v);
       
        nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
       
        if ((t == PLINC_TYPE_ARRAY) || (t == PLINC_TYPE_STRING)) {
            nv.Val.Int = PLINC_SIZE(*v);
        } else if (t == PLINC_TYPE_DICT) {
            nv.Val.Int = ((PlincDict *)(v->Val.Ptr))->Len;
        } else {
            return i->typecheck;
        }

        PLINC_OPPOP(i);
        PLINC_OPPUSH(i, nv);
        
        return NULL;
    }
}



static const PlincOp ops[] = {
    {"put",         op_put},
    {"length",      op_length},

    {NULL,          NULL}
};



void
PlincInitPolymorphOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

/* $Endicor: arith.c,v 1.2 1999/01/18 00:54:54 tsarna Exp tsarna $ */

#include <plinc/interp.h>

#include <stdlib.h>

static void *
op_vmstatus(PlincInterp *i)
{
    PlincVal v;
    
    if (!PLINC_OPSTACKROOM(i, 3)) {
        return i->stackoverflow;
    } else {
        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
        
        v.Val.Int = i->SaveLevel;
        PLINC_OPPUSH(i, v);

        v.Val.Int = ((char *)(i->Heap->HeapHeader->Top))
            - ((char *)(i->Heap->HeapHeader));
        PLINC_OPPUSH(i, v);

        v.Val.Int = i->Heap->HeapHeader->Len;
        PLINC_OPPUSH(i, v);

        return NULL;
    }
}



static PlincOp ops[] = {
    {"vmstatus",    op_vmstatus},

    {NULL,          NULL}
};



void
PlincInitVMOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

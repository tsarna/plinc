/* $Endicor: array.c,v 1.5 1999/01/18 22:05:42 tsarna Exp tsarna $ */


#include <plinc/interp.h>

#include <string.h>


typedef struct _PlincString PlincString;
struct _PlincString {
    PlincUInt   Flags;
};



void *
PlincNewString(PlincHeap *h, PlincUInt size)
{
    PlincHeapHeader *hh = h->HeapHeader;
    PlincString *r = NULL;

    r = PlincAllocHeap(h, sizeof(PlincString) + size);

    if (r) {
        PLINC_LINK(r) = hh->Objects;
        hh->Objects = r;

        r->Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | size;

        memset(++r, 0, size);
    }

    return r;
}


                
static void *
op_string(PlincInterp *i)
{
    PlincVal *v, nv;
    void *r;
    int j;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) == PLINC_TYPE_INT) {
            j = v->Val.Int;
            if ((j >= 0) && (j < PLINC_MAXLEN)) {
                r = PlincNewString(i->Heap, j);
                if (r) {
                    nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | j;
                    nv.Val.Ptr = r;
                    
                    PLINC_OPPOP(i);
                    PLINC_OPPUSH(i, nv);
                } else {
                    return i->VMerror;
                }
            } else {
                return i->rangecheck;
            }
        } else {
            return i->typecheck;
        }
    }

    return NULL;
}



static PlincOp ops[] = {
    {"string",      op_string},

    {NULL,          NULL}
};



void
PlincInitStringOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

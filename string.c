/* $Endicor: string.c,v 1.1 1999/01/19 23:10:35 tsarna Exp tsarna $ */


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



static void *
op_cvn(PlincInterp *i)
{
    PlincVal *v, nv;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) != PLINC_TYPE_STRING) {
            return i->typecheck;
        } else {
            nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_NAME;
            nv.Val.Ptr = PlincName(i->Heap, v->Val.Ptr,
                min(PLINC_SIZE(*v), PLINC_MAXNAMELEN));

            if (nv.Val.Ptr) {
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, nv);
                
                return NULL;
            } else {
                return i->VMerror;
            }
        }
    }
}



static PlincOp ops[] = {
    {"string",      op_string},
    {"cvn",         op_cvn},

    {NULL,          NULL}
};



void
PlincInitStringOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

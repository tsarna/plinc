/* $Endicor: array.c,v 1.3 1999/01/18 00:54:54 tsarna Exp tsarna $ */


#include <plinc/array.h>
#include <plinc/interp.h>

#include <stdlib.h>


void *
PlincNewArray(PlincHeap *h, PlincUInt size)
{
    PlincHeapHeader *hh = h->HeapHeader;
    PlincArray *r = NULL;
    PlincVal *v;

    r = PlincAllocHeap(h, sizeof(PlincArray) + sizeof(PlincVal) * size);

    if (r) {
        PLINC_LINK(r) = hh->Objects;
        hh->Objects = r;

        r->Flags = PLINC_ATTR_LIT | PLINC_TYPE_ARRAY | size;

        v = (PlincVal *)(++r);

        while (size) {
            v->Flags = PLINC_ATTR_LIT | PLINC_TYPE_NULL;

            v++; size--;
        }
    }

    return r;
}



static void *
op_array(PlincInterp *i)
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
                r = PlincNewArray(i->Heap, j);
                if (r) {
                    nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_ARRAY | j;
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
op_astore(PlincInterp *i)
{
    PlincVal *v;
    int l;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        if (PLINC_TYPE(*v) != PLINC_TYPE_ARRAY) {
            return i->typecheck;
        } else {
            if (!PLINC_CAN_WRITE(*v)) {
                return i->invalidaccess;
            } else {
                l = PLINC_SIZE(*v);
                if (!PLINC_OPSTACKHAS(i, l + 1)) {
                    return i->stackunderflow;
                } else if (l) {
                    /* XXX COW the array here... */

                    memcpy(v->Val.Ptr, &PLINC_OPTOPDOWN(i, l),
                        sizeof(PlincVal) * l);

                    PlincClearN(i, l + 1);
                    PLINC_OPPUSH(i, *v);
                }
            }
        }
    }
    
    return NULL;
}



static PlincOp ops[] = {
    {"array",       op_array},

    {"astore",      op_astore},

    {NULL,          NULL}
};



void
PlincInitArrayOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

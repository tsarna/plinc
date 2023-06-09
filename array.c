/* $Endicor: array.c,v 1.9 1999/01/24 03:47:42 tsarna Exp $ */


#include <plinc/interp.h>

#include <stdlib.h>
#include <string.h>



void *
PlincNewArray(PlincHeap *h, PlincUInt size)
{
    PlincVal *v, *r = NULL;

    r = PlincAllocHeap(h, sizeof(PlincVal) * size);

    if (r) {
        r->Flags = PLINC_ATTR_LIT | PLINC_TYPE_ARRAY | size;

        v = r;

        while (size) {
            v->Flags = PLINC_ATTR_LIT | PLINC_TYPE_NULL;

            v++; size--;
        }
    }

    return r;
}



void *
PlincArrayVal(PlincInterp *i, PlincVal *a, PlincVal *ret)
{
    void *r = NULL;
    PlincVal *v;
    int s;
    
    if (!PLINC_CAN_EXEC(*a)) {
        r = i->invalidaccess;
    } else if ((s = PLINC_SIZE(*a))) {
        r = i;
        v = (PlincVal *)(a->Val.Ptr);
        *ret = *v;
        a->Val.Ptr = ++v;
        a->Flags &= ~PLINC_SIZE_MASK;
        a->Flags |= (s - 1);
    }

    return r;
}


                
void *
PlincPutArray(PlincInterp *i, PlincVal *a, PlincUInt ix, PlincVal *v)
{
    PLINC_INCREF_VAL(*v);
    PLINC_DECREF_VAL(a[ix]);
    a[ix] = *v;
    
    return NULL;
}



static void *
op_rsquare(PlincInterp *i)
{
    PlincVal nv;
    PlincInt l;

    l = PlincCountToMark(i);
    if (l < 0) {
        return i->unmatchedmark;
    } else {
        nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_ARRAY | l;
        nv.Val.Ptr = PlincNewArray(i->Heap, l);

        if (nv.Val.Ptr) {
            while (l) {
                l--;
                PlincPutArray(i, nv.Val.Ptr, l, &PLINC_OPTOPDOWN(i, 0));
                PLINC_OPPOP(i);
            }
            PLINC_OPPOP(i); /* remove mark */
            PLINC_OPPUSH(i, nv);
        } else {
            return i->VMerror;
        }
    }
    
    return NULL;
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
op_aload(PlincInterp *i)
{
    PlincVal v;
    int l;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = PLINC_OPTOPDOWN(i, 0);
        if (PLINC_TYPE(v) != PLINC_TYPE_ARRAY) {
            return i->typecheck;
        } else if (!PLINC_CAN_READ(v)) {
            return i->invalidaccess;
        } else if (!PLINC_OPSTACKROOM(i, PLINC_SIZE(v))) {
            return i->stackoverflow;
        } else {
            l = PLINC_SIZE(v);
            
            memcpy(&PLINC_OPTOPDOWN(i, 0), v.Val.Ptr,
                   sizeof(PlincVal) * l);

            i->OpStack.Len += l;
                   
            while (l) {
                l--;
                
                PLINC_INCREF_VAL(PLINC_OPTOPDOWN(i, l));
            }

            PLINC_OPTOPDOWN(i, 0) = v;
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
                    /* XXX ARRAYPUT */

                    memcpy(v->Val.Ptr, &PLINC_OPTOPDOWN(i, l),
                        sizeof(PlincVal) * l);

                    i->OpStack.Len -= (l + 1);
                    
                    PLINC_OPPUSH(i, *v);
                }
            }
        }
    }
    
    return NULL;
}



static const PlincOp ops[] = {
    {op_rsquare,    "]"},
    {op_array,      "array"},
    {op_aload,      "aload"},
    {op_astore,     "astore"},

    {NULL,          NULL}
};



void
PlincInitArrayOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

/* $Endicor: polymorph.c,v 1.5 1999/01/24 03:47:42 tsarna Exp $ */

#include <plinc/interp.h>

#include <stdlib.h>



static void *
op_copy(PlincInterp *i)
{
    PlincVal *v0   /*, *v1*/;
    int j;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v0 = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v0) == PLINC_TYPE_INT) {
            j = v0->Val.Int;
            if (j < 0) {
                return i->rangecheck;
            } else if (j == 0) {
                PLINC_OPPOP(i);
            } else if (!PLINC_OPSTACKROOM(i, j - 1)) {
                return i->stackoverflow;
            } else if (!PLINC_OPSTACKHAS(i, j + 1)) {
                return i->stackunderflow;
            } else {
                memcpy(&PLINC_OPTOPDOWN(i, 0),
                       &PLINC_OPTOPDOWN(i, j),
                       sizeof(PlincVal) * j);
                
                i->OpStack.Len += (j - 1);
                
                while (j) {
                    j--;
                    PLINC_INCREF_VAL(PLINC_OPTOPDOWN(i, j));
                }
            }
            
            return NULL;
        } else {
            return i->typecheck;
        }
    }
}



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
        } else {
            return i->typecheck;
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
op_get(PlincInterp *i)
{
    PlincVal *v0, *v1, v;
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v0 = &PLINC_OPTOPDOWN(i, 1);
        v1 = &PLINC_OPTOPDOWN(i, 0);
        r = i->typecheck;

        if (PLINC_TYPE(*v0) == PLINC_TYPE_DICT) {
            r = PlincGetDict(i, (PlincDict *)(v0->Val.Ptr), v1, &v);
        } else {
            return i->typecheck;
        }

        if (!r) {
            PLINC_OPPOP(i);
            PLINC_OPPOP(i);

            PLINC_OPPUSH(i, v);
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
    {op_copy,       "copy"},
    {op_put,        "put"},
    {op_get,        "get"},
    {op_length,     "length"},

    {NULL,          NULL}
};



void
PlincInitPolymorphOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

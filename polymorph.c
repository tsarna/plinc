#include <plinc/interp.h>

#include <stdlib.h>
#include <string.h>



static void *
op_copy(PlincInterp *i)
{
    PlincVal *v0, *v1, v;
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
        } else if (PLINC_TYPE(*v0) == PLINC_TYPE_STRING) {
            v1 = &PLINC_OPTOPDOWN(i, 1);
            
            if (PLINC_TYPE(*v1) != PLINC_TYPE_STRING) {
                return i->typecheck;
            } else if (!PLINC_CAN_READ(*v1) || !PLINC_CAN_WRITE(*v0)) {
                return i->invalidaccess;
            } else if (PLINC_SIZE(*v1) > PLINC_SIZE(*v0)) {
                return i->rangecheck;
            } else {
                memcpy(v0->Val.Ptr, v1->Val.Ptr, PLINC_SIZE(*v1));
                v.Flags = v1->Flags;
                v.Val.Ptr = v0->Val.Ptr;
                
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, v);
                
                return NULL;
            }
        } else if (PLINC_TYPE(*v0) == PLINC_TYPE_ARRAY) {
            v1 = &PLINC_OPTOPDOWN(i, 1);
            
            if (PLINC_TYPE(*v1) != PLINC_TYPE_ARRAY) {
                return i->typecheck;
            } else if (!PLINC_CAN_READ(*v1) || !PLINC_CAN_WRITE(*v0)) {
                return i->invalidaccess;
            } else if (PLINC_SIZE(*v1) > PLINC_SIZE(*v0)) {
                return i->rangecheck;
            } else {
                memcpy(v0->Val.Ptr, v1->Val.Ptr,
                    PLINC_SIZE(*v1) * sizeof(PlincVal));

                v.Flags = v1->Flags;
                v.Val.Ptr = v0->Val.Ptr;
                
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, v);
                
                return NULL;
            }
        } else {
            return i->typecheck;
        }
    }
}



static void *
op_get(PlincInterp *i)
{
    PlincVal *v0, *v1, v;
    void *r = NULL;
    
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v0 = &PLINC_OPTOPDOWN(i, 1);
        v1 = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v0) == PLINC_TYPE_DICT) {
            r = PlincGetDict(i, (PlincDict *)(v0->Val.Ptr), v1, &v);
        } else if (PLINC_TYPE(*v0) == PLINC_TYPE_ARRAY) {
            if (!PLINC_CAN_READ(*v0)) {
                return i->invalidaccess;
            } else if (PLINC_TYPE(*v1) != PLINC_TYPE_INT) {
                return i->typecheck;
            } else if ((v1->Val.Int < 0) || (v1->Val.Int >= PLINC_SIZE(*v0))) {
                return i->rangecheck;
            } else {
                v = ((PlincVal *)v0->Val.Ptr)[v1->Val.Int];
                PLINC_INCREF_VAL(v);
            }
        } else if (PLINC_TYPE(*v0) == PLINC_TYPE_STRING) {
            if (!PLINC_CAN_READ(*v0)) {
                return i->invalidaccess;
            } else if (PLINC_TYPE(*v1) != PLINC_TYPE_INT) {
                return i->typecheck;
            } else if ((v1->Val.Int < 0) || (v1->Val.Int >= PLINC_SIZE(*v0))) {
                return i->rangecheck;
            } else {
                v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
                v.Val.Int = ((char *)v0->Val.Ptr)[v1->Val.Int];
            }
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
        } else if (PLINC_TYPE(*v2) == PLINC_TYPE_STRING) {
            if (!PLINC_CAN_WRITE(*v2)) {
                return i->invalidaccess;
            } else if ((PLINC_TYPE(*v1) != PLINC_TYPE_INT)
                   ||  (PLINC_TYPE(*v0) != PLINC_TYPE_INT)) {
                return i->typecheck;
            } else if ((v1->Val.Int < 0) || (v1->Val.Int >= PLINC_SIZE(*v2))) {
                return i->rangecheck;
            } else {
                ((char *)(v2->Val.Ptr))[v1->Val.Int] = v0->Val.Int;

                r = NULL;
            }
        } else if (PLINC_TYPE(*v2) == PLINC_TYPE_ARRAY) {
            if (!PLINC_CAN_WRITE(*v2)) {
                return i->invalidaccess;
            } else if (PLINC_TYPE(*v1) != PLINC_TYPE_INT) {
                return i->typecheck;
            } else if ((v1->Val.Int < 0) || (v1->Val.Int >= PLINC_SIZE(*v2))) {
                return i->rangecheck;
            } else {
                PLINC_INCREF_VAL(*v0);

                ((PlincVal *)(v2->Val.Ptr))[v1->Val.Int] = *v0;
                
                r = NULL;
            }
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



static void *
op_getinterval(PlincInterp *i)
{
    PlincVal *v0, *v1, *v2;
    PlincInt j, l;
    
    if (!PLINC_OPSTACKHAS(i, 3)) {
        return i->stackunderflow;
    } else {
        v0 = &PLINC_OPTOPDOWN(i, 2);
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);
        
        if ((PLINC_TYPE(*v1) != PLINC_TYPE_INT)
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_INT)
        ||  ((PLINC_TYPE(*v0) != PLINC_TYPE_ARRAY)
          && (PLINC_TYPE(*v0) != PLINC_TYPE_STRING))
        ) {
            return i->typecheck;
        } else {
            j = v1->Val.Int;
            l = v2->Val.Int;
            
            if ((j < 0) || (l < 0) || ((j + l) > PLINC_SIZE(*v0))) {
                return i->rangecheck;
            } else {
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
                
                v0->Flags &= ~PLINC_SIZE_MASK;
                v0->Flags |= l;
                
                if (PLINC_TYPE(*v0) == PLINC_TYPE_STRING) {
                    v0->Val.Ptr = ((char *)(v0->Val.Ptr)) + j;
                } else {
                    v0->Val.Ptr = ((PlincVal *)(v0->Val.Ptr)) + j;
                }
              
                return NULL;
            }
        }
    }
}



static const PlincOp ops[] = {
    {op_copy,           "copy"},
    {op_get,            "get"},
    {op_put,            "put"},
    {op_length,         "length"},
    {op_getinterval,    "getinterval"},

    {NULL,          NULL}
};



void
PlincInitPolymorphOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

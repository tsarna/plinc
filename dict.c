#include <plinc/interp.h>

#include <stdlib.h>


void *
PlincNewDict(PlincHeap *h, PlincUInt size)
{
    PlincHeapHeader *hh = h->HeapHeader;
    PlincDict *r = NULL;
    int i;

    r = PlincAllocHeapLinked(h, sizeof(PlincDict)
        + sizeof(PlincDictEnt) * (size - 1));

    if (r) {
        PLINC_LINK(r) = hh->Objects;
        hh->Objects = r;

        r->Flags = PLINC_ATTR_LIT | PLINC_TYPE_DICT | size;
        r->Len = 0;
        r->MaxLen = size;

        for (i = 0; i < size; i++) {
            r->Vals[i].Key.Flags = PLINC_ATTR_LIT | PLINC_TYPE_NULL;
            r->Vals[i].Val.Flags = PLINC_ATTR_LIT | PLINC_TYPE_NULL;
        }
    }

    return r;
}



PlincUInt
PlincHash(PlincUInt r)
{
    r = r ^ 0x876AE319;
    r = (r >> 16) ^ (r & 0xFFFF);
    r = r ^ (r * 251);
    r = (r >> 16) ^ (r & 0xFFFF);
    r = r ^ (r * 65521);
    r = (r >> 16) ^ (r & 0xFFFF);
    r = r ^ (r * 4099);
    r = (r >> 16) ^ (r & 0xFFFF);

    return r;
}


PlincUInt
PlincHashVal(PlincVal *v)
{
    return PlincHash((PlincUInt)(v->Val.Ptr));
}


void *
PlincPutDict(PlincInterp *i, PlincDict *d, PlincVal *key, PlincVal *val)
{
    void *r = NULL;
    PlincUInt j;
    PlincVal v;

    if (!PLINC_CAN_WRITE(*d)) {
        r = i->invalidaccess;
    } else if (d->Len >= d->MaxLen) {
        r = i->dictfull;
    } else {
        d->Len++;

        if (PLINC_TYPE(*key) == PLINC_TYPE_STRING) {
            v.Val.Ptr = PlincName(i->Heap, key->Val.Ptr, PLINC_SIZE(*key));

            if (v.Val.Ptr) {
                v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_NAME;
                key = &v;
            } else {
                return i->VMerror;
            }
        }

        j = PlincHashVal(key) % (d->MaxLen);

        while (!PLINC_IS_NULL(d->Vals[j].Key) && 
            !PlincEqual(&(d->Vals[j].Key), key)) {

            j++;
            
            if (j >= d->MaxLen) {
                j = 0;
            }
        }

        if (PLINC_IS_NULL(d->Vals[j].Key)) {
            PLINC_INCREF_VAL(*key);
            d->Vals[j].Key = *key;
        }
        
        PLINC_INCREF_VAL(*val);
        PLINC_DECREF_VAL(d->Vals[j].Val);
        d->Vals[j].Val = *val;
    }
    
    return r;
}



void *
PlincPutDictName(PlincInterp *i, PlincDict *d, void *key, PlincVal *val)
{
    PlincVal k;
    
    k.Flags = PLINC_ATTR_LIT | PLINC_TYPE_NAME;
    k.Val.Ptr = key;
    
    return PlincPutDict(i, d, &k, val);
}



void *
PlincGetDict(PlincInterp *i, PlincDict *d, PlincVal *key, PlincVal *val)
{
    void *r = i->undefined;
    PlincUInt j, oj;
    PlincVal v;

    if (!PLINC_CAN_READ(*d)) {
        r = i->invalidaccess;
    } else {
        if (PLINC_TYPE(*key) == PLINC_TYPE_STRING) {
            v.Val.Ptr = PlincName(i->Heap, key->Val.Ptr, PLINC_SIZE(*key));

            if (v.Val.Ptr) {
                v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_NAME;
                key = &v;
            } else {
                return i->VMerror;
            }
        }

        oj = j = PlincHashVal(key) % (d->MaxLen);

        while (!PLINC_IS_NULL(d->Vals[j].Key)) {
            if (PlincEqual(&(d->Vals[j].Key), key)) {
                *val = d->Vals[j].Val;
                
                return NULL;
            }

            j++;
            
            if (j >= d->MaxLen) {
                j = 0;
            }
            
            if (j == oj) {
                break;
            }
        }
    }

    return r;
}



void *
PlincLoadDict(PlincInterp *i, PlincVal *key, PlincVal *val)
{
    void *r;
    int j;
    
    for (j = 0; j < i->DictStack.Len; j++) {
        r = PlincGetDict(i, PLINC_TOPDOWN(i->DictStack, j).Val.Ptr, key, val);
        if (r != i->undefined) {
            return r;
        }
    }
    
    return i->undefined;
}



static void *
op_dict(PlincInterp *i)
{
    PlincVal *v, nv;
    PlincDict *d;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) != PLINC_TYPE_INT) {
            return i->typecheck;
        } else if (v->Val.Int > PLINC_MAXLEN) {
            return i->limitcheck;
        } else {
            d = PlincNewDict(i->Heap, v->Val.Int);
            
            if (d) {
                nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_DICT | v->Val.Int;
                nv.Val.Ptr = d;
                
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, nv);
                
                return NULL;
            } else {
                return i->VMerror;
            }
        }
    }
}



static void *
op_maxlength(PlincInterp *i)
{
    PlincVal *v, nv;
    PlincDict *d;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) == PLINC_TYPE_DICT) {
            d = (PlincDict *)(v->Val.Ptr);
            
            if (PLINC_CAN_READ(*d)) {
                nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
                nv.Val.Int = d->MaxLen;
                
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, nv);
            } else {
                return i->invalidaccess;
            }
        } else {
            return i->typecheck;
        }
    }

    return NULL;
}



static void *
op_begin(PlincInterp *i)
{
    PlincDict *d;
    PlincVal *v;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        if (PLINC_TYPE(*v) != PLINC_TYPE_DICT) {
            return i->typecheck;
        } else if (!PLINC_STACKROOM(i->DictStack, 1)) {
            return i->dictstackoverflow;
        } else { 
            PLINC_PUSH(i->DictStack, *v);
            PLINC_OPPOP(i);

            d = v->Val.Ptr;

            return NULL;
        }
    }
}



static void *
op_end(PlincInterp *i)
{
    if (!PLINC_STACKHAS(i->DictStack, i->DictStack.MinLen + 1)) {
        return i->dictstackunderflow;
    } else {
        PLINC_POP(i->DictStack);
        
        return NULL;
    }
}



static void *
op_def(PlincInterp *i)
{
    PlincVal *k, *v;
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        k = &PLINC_OPTOPDOWN(i, 1);
        v = &PLINC_OPTOPDOWN(i, 0);

        r = PlincPutDict(i, PLINC_TOPDOWN(i->DictStack, 0).Val.Ptr, k, v);
        if (!r) {
            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
        }
    }
    
    return r;
}



static void *
op_load(PlincInterp *i)
{
    PlincVal v;
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        r = PlincLoadDict(i, &PLINC_OPTOPDOWN(i, 0), &v);
        if (r) {
            return r;
        } else {
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);
            
            return NULL;
        }
    }
}



static void *
op_known(PlincInterp *i)
{
    PlincVal *v0, *v1, v;
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v0 = &PLINC_OPTOPDOWN(i, 1);
        v1 = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v0) != PLINC_TYPE_DICT) {
            return i->typecheck;
        } else {
            r = PlincGetDict(i, (PlincDict *)(v0->Val.Ptr), v1, &v);
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;

	    if (!r || (r == i->undefined)) {
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);

                v.Val.Int = !r;

                PLINC_OPPUSH(i, v);

                return NULL;
            } else {
                return r;
            }
        }
    }
}



static void *
op_where(PlincInterp *i)
{
    PlincDict *d;
    PlincVal v;
    void *r;
    int j;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        for (j = 0; j < i->DictStack.Len; j++) {
            d = PLINC_TOPDOWN(i->DictStack, j).Val.Ptr;
            r = PlincGetDict(i, d, &PLINC_OPTOPDOWN(i, 0), &v);
            if (r != i->undefined) {
                PLINC_OPPOP(i);

                v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_DICT | PLINC_SIZE(*d);
                v.Val.Ptr = d;

                PLINC_OPPUSH(i, v);

                v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
                v.Val.Int = TRUE;

                PLINC_OPPUSH(i, v);
                
                return NULL;
            }
        }
        
        PLINC_OPPOP(i);

        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
        v.Val.Int = FALSE;

        PLINC_OPPUSH(i, v);
        
        return NULL;
    }
}



static void *
op_currentdict(PlincInterp *i)
{
    if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        PLINC_OPPUSH(i, PLINC_TOPDOWN(i->DictStack, 0));

        return NULL;
    }
}



static const PlincOp ops[] = {
    {op_dict,           "dict"},
    {op_maxlength,      "maxlength"},
    {op_begin,          "begin"},
    {op_end,            "end"},
    {op_def,            "def"},
    {op_load,           "load"},
    {op_known,          "known"},
    {op_where,          "where"},
    {op_currentdict,    "currentdict"},

    {NULL,              NULL}
};



void
PlincInitDictOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

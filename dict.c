/* $Endicor: dict.c,v 1.6 1999/01/18 00:54:54 tsarna Exp tsarna $ */


#include <plinc/interp.h>

#include <stdlib.h>


void *
PlincNewDict(PlincHeap *h, PlincUInt size)
{
    PlincHeapHeader *hh = h->HeapHeader;
    PlincDict *r = NULL;
    int i;

    r = PlincAllocHeap(h, sizeof(PlincDict)
        + sizeof(PlincDictEnt) * (size - 1));

    if (r) {
        PLINC_LINK(r) = hh->Objects;
        hh->Objects = r;

        r->Flags = PLINC_ATTR_LIT | PLINC_TYPE_DICT | size;
        r->Len = 0;
        r->MaxLen = size;

        for (i = 0; i < size; i++) {
            r->Vals[i].Key.Flags = PLINC_ATTR_LIT | PLINC_TYPE_NULL;
        }
    }

    return r;
}



PlincUInt
PlincHashPtr(void *p)
{
    PlincUInt r;

    r = (PlincUInt)p;

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


void *
PlincPutDictName(PlincInterp *i, PlincDict *d, void *key, PlincVal *val)
{
    void *r = NULL;
    PlincUInt j;

    if (!PLINC_CAN_WRITE(*d)) {
        r = i->invalidaccess;
    } else if (d->Len < d->MaxLen) {
        d->Len++;

        j = PlincHashPtr(key) % (d->MaxLen);

        while (!PLINC_IS_NULL(d->Vals[j].Key)) {
            j++;
            
            if (j >= d->MaxLen) {
                j = 0;
            }
        }

        d->Vals[j].Key.Flags = PLINC_ATTR_LIT | PLINC_TYPE_NAME;
        d->Vals[j].Key.Val.Ptr = key;
        d->Vals[j].Val = *val;
    } else {
        r = i->dictfull;
    }
    
    return r;
}



void *
PlincGetDict(PlincInterp *i, PlincDict *d, PlincVal *key, PlincVal *val)
{
    void *r = i->undefined;
    PlincUInt j, oj;

    if (!PLINC_CAN_READ(*d)) {
        r = i->invalidaccess;
    } else {
        oj = j = PlincHashPtr(key->Val.Ptr) % (d->MaxLen);

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
                return i->invalidaccess;
            } else {
                nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
                nv.Val.Int = d->MaxLen;
                
                PLINC_OPPUSH(i, nv);
            }
        } else {
            return i->typecheck;
        }
    }

    return NULL;
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



static void *
op_countdictstack(PlincInterp *i)
{
    PlincVal v;
    
    if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
        v.Val.Int = i->DictStack.Len;

        PLINC_OPPUSH(i, v);

        return NULL;
    }
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


static PlincOp ops[] = {
    {"maxlength",       op_maxlength},

    {"load",            op_load},

    {"currentdict",     op_currentdict},
    {"countdictstack",  op_countdictstack},

    {NULL,              NULL}
};



void
PlincInitDictOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

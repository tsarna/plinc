/* $Endicor: dict.c,v 1.3 1999/01/17 04:58:16 tsarna Exp $ */


#include <plinc/interp.h>

#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif


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

fprintf(stderr, "XXX: Key is ");PlincReprVal(i, key);fprintf(stderr, "\n");
    if (!PLINC_CAN_READ(*d)) {
        r = i->invalidaccess;
    } else {
        oj = j = PlincHashPtr(key->Val.Ptr) % (d->MaxLen);

        while (!PLINC_IS_NULL(d->Vals[j].Key)) {
fprintf(stderr, "XXX: Comparing to ");PlincReprVal(i, &(d->Vals[j].Key));fprintf(stderr, "\n");
            if (PlincEqual(&(d->Vals[j].Key), key)) {
                *val = d->Vals[j].Val;
                
fprintf(stderr, "XXX: Match!\n");
                return NULL;
            }

            j++;
            
            if (j >= d->MaxLen) {
fprintf(stderr, "XXX: Dict lookup looping\n");
                j = 0;
            }
            
            if (j == oj) {
fprintf(stderr, "XXX: Back to starting point\n");
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
fprintf(stderr, "XXX: looking n dict @ %p\n", PLINC_TOPDOWN(i->DictStack, j).Val.Ptr);
        r = PlincGetDict(i, PLINC_TOPDOWN(i->DictStack, j).Val.Ptr, key, val);
        if (r != i->undefined) {
            return r;
        }
    }
    
    return i->undefined;
}



#ifdef DEBUG

void
PlincPrintName(PlincInterp *i, void *name)
{
    unsigned char *n = name;
    size_t s;

    s = *n++;

    fprintf(stderr, "%d %.*s", s, (int)s, n);
}



void
PlincPrintDict(PlincInterp *i, PlincDict *d)
{
    int j;
    
    for (j = 0; j < d->MaxLen; j++) {
        if (!PLINC_IS_NULL(d->Vals[j].Key)) {
            PlincPrintName(i, d->Vals[j].Key.Val.Ptr);
            fprintf(stderr, "\n");
        }
    }
}


#endif

/* $Endicor: dict.c,v 1.2 1999/01/14 05:02:15 tsarna Exp $ */


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


#ifdef DEBUG

void
PlincPrintName(PlincInterp *i, void *name)
{
    unsigned char *n = name;
    size_t s;

    s = *n++;

    fprintf(stderr, "%d %.*s", s, s, n);
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

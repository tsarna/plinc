/* $Endicor: array.c,v 1.1 1999/01/12 22:27:34 tsarna Exp tsarna $ */


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
            r->Vals[i].Key = NULL;
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
PlincPutDict(PlincInterp *i, PlincDict *d, void *key, PlincVal *val)
{
    void *r = i->dictfull;
    PlincUInt j;

    if (!PLINC_CAN_WRITE(d)) {
        return i->invalidaccess;
    } else if (d->Len < d->MaxLen) {
        d->Len++;

        j = PlincHashPtr(key) % (d->MaxLen);

        while (d->Vals[j].Key) {
            if (++j >= d->MaxLen) {
                j = 0;
            }
        }

        d->Vals[j].Key = key;
        d->Vals[j].Val = *val;
    } else {
        r = i->dictfull;
    }

    return r;
}

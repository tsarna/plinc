/* $Endicor: interp.c,v 1.1 1999/01/14 01:26:00 tsarna Exp tsarna $ */

#include <plinc/interp.h>

#include <stdlib.h>
#include <stdio.h> /*XXX*/


PlincInterp *
PlincNewInterp(size_t heapsize)
{
    PlincInterp *i;
    int ok = TRUE;
    
    i = malloc(sizeof(PlincInterp));
    if (i) {
        ok = PlincNewStack(&(i->OpStack), 500);
        if (ok) {
            ok = PlincNewStack(&(i->DictStack), 20);
        }
        if (ok) {
            ok = PlincNewStack(&(i->ExecStack), 250);
        }
        if (ok) { 
            i->Heap = PlincNewHeap(heapsize);
            if (!i->Heap) {
                ok = FALSE;
            }
        }
        if (ok) {
            PlincInitErrorNames(i);
            PlincInitVals(i);
        }
                    
    }

    if (!ok) {
        PlincFreeInterp(i);
        i = NULL;
    }
    
    return i;
}



void
PlincFreeInterp(PlincInterp *i)
{
    if (i) {
        PlincFreeStack(&(i->OpStack));
        PlincFreeStack(&(i->DictStack));
        PlincFreeStack(&(i->ExecStack));

        PlincFreeHeap(i->Heap);
    }
}



void
PlincInitErrorNames(PlincInterp *i)
{
    PlincHeap *h = i->Heap;
    
#define DEFNAME(x) PlincName(h, x, strlen(x))
#define DEFERR(x) i->x = DEFNAME(#x);

    DEFERR(dictfull);

    DEFERR(invalidaccess);

    DEFERR(stackoverflow);
    DEFERR(stackunderflow);
}



void
PlincInitVals(PlincInterp *i)
{
    PlincHeap *h = i->Heap;
    PlincDict *d;
    PlincVal v;
    void *name;
    
#define DEFDICT(x, s) name = DEFNAME(#x); d = PlincNewDict(h, s); \
    i->x = d; v.Flags = d->Flags; v.Val.Ptr = d; \
    PlincPutDict(i, i->systemdict, name, &v);

    DEFDICT(systemdict, 50);
    PLINC_PUSH(i->DictStack, v);
    i->DictStack.MinLen = 1;

    DEFDICT(errordict, 50);
    DEFDICT(userdict, 200);

    /* mark, .mark */
    v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_MARK;
    name = DEFNAME("mark");
    PlincPutDict(i, i->systemdict, name, &v);
    name = DEFNAME(".mark");
    PlincPutDict(i, i->systemdict, name, &v);

    /* true, false */
    v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
    v.Val.Int = 1;
    name = DEFNAME("true");
    PlincPutDict(i, i->systemdict, name, &v);
    v.Val.Int = 0;
    name = DEFNAME("false");
    PlincPutDict(i, i->systemdict, name, &v);

    /* null */
    v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_NULL;
    name = DEFNAME("null");
    PlincPutDict(i, i->systemdict, name, &v);

    /* put systemdict on dictstack */
}

/* $Endicor: interp.c,v 1.8 1999/01/18 00:54:54 tsarna Exp tsarna $ */

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
            i->ScanLevel = i->SaveLevel = 0;
            i->GotInterrupt = FALSE;
            
            PlincInitErrorNames(i);
            PlincInitVals(i);
            PlincInitStackOps(i);
            PlincInitPrintOps(i);
            PlincInitArithOps(i);
            PlincInitArrayOps(i);
            PlincInitDictOps(i);
            PlincInitRelationalOps(i);
            PlincInitControlOps(i);
            PlincInitVMOps(i);
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
#define DEFERR(x) i->x = DEFNAME(#x); \
  fprintf(stderr, "ERR %s @ %p\n", #x, i->x);

    DEFERR(dictfull);
    DEFERR(dictstackoverflow);
    DEFERR(dictstackunderflow);
    DEFERR(execstackoverflow);

    DEFERR(interrupt);
    DEFERR(invalidaccess);

    DEFERR(rangecheck);
    DEFERR(stackoverflow);
    DEFERR(stackunderflow);
    DEFERR(syntaxerror);

    DEFERR(typecheck);
    DEFERR(undefined);

    DEFERR(undefinedresult);
    DEFERR(unmatchedmark);
    DEFERR(unregistered);
    DEFERR(VMerror);
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
    PlincPutDictName(i, i->systemdict, name, &v); \
    fprintf(stderr, "DICT %s @ %p\n", #x, d);

    DEFDICT(systemdict, 50);
    PLINC_PUSH(i->DictStack, v);
    i->DictStack.MinLen = 1;

    DEFDICT(errordict, 50);
    DEFDICT(userdict, 200);

    /* mark, .mark */
    v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_MARK;
    name = DEFNAME("mark");
    PlincPutDictName(i, i->systemdict, name, &v);
    name = DEFNAME("[");
    PlincPutDictName(i, i->systemdict, name, &v);
    i->LeftBracket = name;
    name = DEFNAME("<<");
    PlincPutDictName(i, i->systemdict, name, &v);
    i->LeftAngleAngle = name;
    
    /* true, false */
    v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
    v.Val.Int = 1;
    name = DEFNAME("true");
    PlincPutDictName(i, i->systemdict, name, &v);
    v.Val.Int = 0;
    name = DEFNAME("false");
    PlincPutDictName(i, i->systemdict, name, &v);

    /* null */
    v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_NULL;
    name = DEFNAME("null");
    PlincPutDictName(i, i->systemdict, name, &v);

    /* define some names */
    i->RightBracket = DEFNAME("]");
    i->LeftBrace = DEFNAME("{");
    i->RightBrace = DEFNAME("}");
    i->RightAngleAngle = DEFNAME(">>");
}



void
PlincInitOps(PlincInterp *i, PlincOp *o)
{
    PlincHeap *h = i->Heap;
    PlincVal v;
    char *name;
    
    v.Flags = PLINC_TYPE_OP;
    
    while (o->Name) {
        name = DEFNAME(o->Name);
        v.Val.Op = o;
    
        PlincPutDictName(i, i->systemdict, name, &v);

        o++;
    }
}

/* $Endicor: interp.c,v 1.23 1999/01/27 03:49:44 tsarna Exp tsarna $ */

#include <plinc/interp.h>

#include <stdlib.h>
#include <stdio.h> /*XXX*/

#include "defs.h"


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
            i->DictStack.MinLen = 1;
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
            i->State = i->Seed = 1;
            i->GotInterrupt = FALSE;
            
            PlincInitErrorNames(i);
            PlincInitTypeNames(i);
            PlincInitVals(i);
            PlincInitStackOps(i);
            PlincInitTypeOps(i);
            PlincInitPrintOps(i);
            PlincInitArithOps(i);
#ifdef WITH_REAL
            PlincInitRealOps(i);
#endif
            PlincInitArrayOps(i);
            PlincInitStringOps(i);
            PlincInitDictOps(i);
            PlincInitRelationalOps(i);
            PlincInitControlOps(i);
            PlincInitLoopOps(i);
            PlincInitPolymorphOps(i);
            PlincInitVMOps(i);
            
            if (PlincExecStr(i, early_defs)) {
                ok = FALSE;
            }
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
    DEFERR(dictstackoverflow);
    DEFERR(dictstackunderflow);
    DEFERR(execstackoverflow);

    DEFERR(interrupt);
    DEFERR(invalidaccess);
    DEFERR(invalidexit);
    DEFERR(invalidstop);
    DEFERR(invalidfileaccess);

    DEFERR(ioerror);
    DEFERR(limitcheck);
    DEFERR(nocurrentpoint);
    DEFERR(rangecheck);
    DEFERR(stackoverflow);
    DEFERR(stackunderflow);
    DEFERR(syntaxerror);
    DEFERR(timeout);
    DEFERR(typecheck);
    DEFERR(undefined);
    DEFERR(undefinedfilename);
    DEFERR(undefinedresult);
    DEFERR(unmatchedmark);
    DEFERR(unregistered);
    DEFERR(VMerror);
}



void
PlincInitTypeNames(PlincInterp *i)
{
    PlincHeap *h = i->Heap;
    
#define DEFTYPENAME(x) i->x = DEFNAME(#x);

    DEFTYPENAME(integertype);
    DEFTYPENAME(realtype);
    DEFTYPENAME(booleantype);
    DEFTYPENAME(arraytype);
    DEFTYPENAME(stringtype);
    DEFTYPENAME(nametype);
    DEFTYPENAME(dicttype);
    DEFTYPENAME(operatortype);
    DEFTYPENAME(filetype);
    DEFTYPENAME(marktype);
    DEFTYPENAME(nulltype);
    DEFTYPENAME(savetype);
    DEFTYPENAME(fonttype);
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
    PlincPutDictName(i, i->systemdict, name, &v);

    DEFDICT(systemdict, 100);
    PLINC_PUSH(i->DictStack, v);
    i->DictStack.MinLen = 1;

    DEFDICT(errordict, 50);

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
PlincInitOps(PlincInterp *i, const PlincOp *o)
{
    PlincHeap *h = i->Heap;
    PlincVal v;
    char *name;
    
    v.Flags = PLINC_TYPE_OP;
    
    while (o->Name) {
        name = DEFNAME(o->Name);
        v.Val.Op = (PlincOp *)o;
    
        PlincPutDictName(i, i->systemdict, name, &v);

        o++;
    }
}

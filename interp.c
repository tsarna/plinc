#include <plinc/interp.h>

#include <stdlib.h>
#include <string.h>

#include "defs.h"


int
PlincInitInterp(PlincInterp *i, int opstack, int dictstack, int execstack)
{
    /* requires an interpreter with an initialized heap pointer */
    
    int ok = TRUE;
    
    ok = PlincAllocStack(i->Heap, &(i->OpStack), 500);
    if (ok) {
        ok = PlincAllocStack(i->Heap, &(i->DictStack), 20);
        i->DictStack.MinLen = 1;
    }
    if (ok) {
        ok = PlincAllocStack(i->Heap, &(i->ExecStack), 250);
    }
    if (ok) {
        i->ScanLevel = i->SaveLevel = 0;
        i->RandState = 1;
        i->GotInterrupt = FALSE;
        
#ifdef WITH_SIMPLE_CTM
        i->DefaultMatrix = PlincDefaultMatrix;
        i->CTM = i->DefaultMatrix;
#endif

        PlincInitErrorNames(i);
        PlincInitTypeNames(i);
        PlincInitVals(i);
        PlincInitStackOps(i);
        PlincInitTypeOps(i);
        PlincInitPrintOps(i);
        PlincInitArithOps(i);
        PlincInitCvtOps(i);
#ifdef WITH_REAL
        PlincInitRealOps(i);
#endif
#ifdef WITH_MATRIX
        PlincInitMatrixOps(i);
#endif
        PlincInitArrayOps(i);
        PlincInitStringOps(i);
        PlincInitDictOps(i);
        PlincInitRelationalOps(i);
        PlincInitControlOps(i);
        PlincInitLoopOps(i);
        PlincInitFileOps(i);
        PlincInitPolymorphOps(i);
        PlincInitVMOps(i);
            
        if (PlincExecStr(i, early_defs)) {
            ok = FALSE;
        }
    }

    return ok;
}



PlincInterp *
PlincNewInterp(size_t heapsize)
{
    PlincInterp *i = NULL;
    PlincHeap *h;
    int ok = FALSE;
    
    h = PlincNewHeap(heapsize);
    if (h) {
        i = PlincAllocHeap(h, sizeof(PlincInterp));
    }
    if (i) {
        i->Heap = h;
        ok = PlincInitInterp(i, 500, 20, 250);
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
    DEFERR(invalidfont);
    DEFERR(invalidrestore);
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

    DEFDICT(systemdict, 192);
    PLINC_PUSH(i->DictStack, v);
    i->DictStack.MinLen = 2; /* next dict pushed will be unpoppable! */

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

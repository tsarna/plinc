/* $Endicor: interp.h,v 1.8 1999/01/18 00:54:54 tsarna Exp tsarna $ */

#ifndef PLINC_INTERP_H
#define PLINC_INTERP_H


#include <plinc/heap.h>
#include <plinc/stack.h>


typedef struct _PlincDictEnt PlincDictEnt;
struct _PlincDictEnt {
    PlincVal    Key;
    PlincVal    Val;
};


typedef struct _PlincDict PlincDict;
struct _PlincDict {
    PlincUInt       Flags;
    PlincUInt       Len;
    PlincUInt       MaxLen;
    PlincDictEnt    Vals[1];
};


typedef struct _PlincInterp PlincInterp;
struct _PlincInterp {
    PlincHeap      *Heap;

    /* stacks */
    PlincStack      OpStack;
    PlincStack      DictStack;
    PlincStack      ExecStack;

    int             ScanLevel;
    int             SaveLevel;
    
    /* commonly needed names */
    void           *LeftBracket;
    void           *RightBracket;
    void           *LeftBrace;
    void           *RightBrace;
    void           *LeftAngleAngle;
    void           *RightAngleAngle;
    
    /* Dicts */
    PlincDict      *systemdict;
    PlincDict      *userdict;
    PlincDict      *errordict;
    
    /* errors */
    void           *dictfull;
    void           *dictstackoverflow;
    void           *dictstackunderflow;
    void           *execstackoverflow;
    
    void           *interrupt;
    void           *invalidaccess;

    void           *rangecheck;
    void           *stackunderflow;
    void           *stackoverflow;
    void           *syntaxerror;

    void           *typecheck;
    void           *undefined;

    void           *undefinedresult;
    void           *unmatchedmark;
    void           *unregistered;
    void           *VMerror;

    /* misc state */
    int             GotInterrupt;    
};


typedef struct _PlincOp PlincOp;
struct _PlincOp {
    char    *Name;
    void  *(*Func)(PlincInterp *);
};


/****************************************/

PlincInterp    *PlincNewInterp(size_t heap);
void            PlincFreeInterp(PlincInterp *i);
void            PlincInitErrorNames(PlincInterp *i);
void            PlincInitVals(PlincInterp *i);

void           *PlincNewDict(PlincHeap *h, PlincUInt size);
PlincUInt       PlincHashPtr(void *p);
void           *PlincPutDictName(PlincInterp *i, PlincDict *d,
                    void *key, PlincVal *val);
void           *PlincGetDict(PlincInterp *i, PlincDict *d,
                    PlincVal *key, PlincVal *val);
void           *PlincLoadDict(PlincInterp *i, PlincVal *key, PlincVal *val);

void            PlincInitOps(PlincInterp *i, PlincOp *o);
void            PlincInitStackOps(PlincInterp *i);
void            PlincInitPrintOps(PlincInterp *i);
void            PlincInitArithOps(PlincInterp *i);
void            PlincInitArrayOps(PlincInterp *i);
void            PlincInitDictOps(PlincInterp *i);
void            PlincInitRelationalOps(PlincInterp *i);
void            PlincInitControlOps(PlincInterp *i);
void            PlincInitVMOps(PlincInterp *i);

int             PlincEqual(PlincVal *v1, PlincVal *v2);
void           *PlincReprVal(PlincInterp *i, PlincVal *v);
void            PlincClearN(PlincInterp *i, int n);

void           *PlincGo(PlincInterp *i);
void           *PlincExecStr(PlincInterp *i, char *s);


#endif /* PLINC_INTERP_H */

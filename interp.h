/* $Endicor: interp.h,v 1.6 1999/01/17 21:04:54 tsarna Exp $ */

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

    void           *execstackoverflow;
    
    void           *invalidaccess;

    void           *rangecheck;
    void           *stackunderflow;
    void           *stackoverflow;
    void           *syntaxerror;

    void           *typecheck;
    void           *undefined;

    void           *unmatchedmark;

    void           *VMerror;
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

int             PlincEqual(PlincVal *v1, PlincVal *v2);
void           *PlincReprVal(PlincInterp *i, PlincVal *v);

void           *PlincGo(PlincInterp *i);
void           *PlincExecStr(PlincInterp *i, char *s);


#endif /* PLINC_INTERP_H */

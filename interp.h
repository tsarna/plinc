/* $Endicor: interp.h,v 1.2 1999/01/14 05:02:15 tsarna Exp tsarna $ */

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

    void           *VMerror;
};


typedef struct _PlincOps PlincOps;
struct _PlincOps {
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
void            PlincPrintName(PlincInterp *i, void *name);
void            PlincPrintDict(PlincInterp *i, PlincDict *d);

void            PlincInitOps(PlincInterp *i, PlincOps *o);
void            PlincInitStackOps(PlincInterp *i);

#endif /* PLINC_INTERP_H */

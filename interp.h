#ifndef PLINC_INTERP_H
#define PLINC_INTERP_H


#include <plinc/heap.h>
#include <plinc/stack.h>


#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif


typedef struct _PlincDictEnt PlincDictEnt;
struct _PlincDictEnt {
    PlincVal    Key;
    PlincVal    Val;
};


typedef struct _PlincDict PlincDict;
struct _PlincDict {
    PlincUInt32     Flags;
    PlincUInt32     Len;
    PlincUInt32     MaxLen;
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
    
    /* random number generator */
    unsigned int    RandState;

    /* commonly needed names */
    void           *LeftBracket;
    void           *RightBracket;
    void           *LeftBrace;
    void           *RightBrace;
    void           *LeftAngleAngle;
    void           *RightAngleAngle;
    
    /* Dicts */
    PlincDict      *systemdict;
    PlincDict      *errordict;
    
    /* errors */
    void           *dictfull;
    void           *dictstackoverflow;
    void           *dictstackunderflow;
    void           *execstackoverflow;
    
    void           *interrupt;
    void           *invalidaccess;
    void           *invalidexit;
    void           *invalidstop;
    void           *invalidfileaccess;
    void           *invalidfont;
    void           *invalidrestore;
    void           *ioerror;
    void           *limitcheck;
    void           *nocurrentpoint;
    void           *rangecheck;
    void           *stackunderflow;
    void           *stackoverflow;
    void           *syntaxerror;
    void           *timeout;
    void           *typecheck;
    void           *undefined;
    void           *undefinedfilename;
    void           *undefinedresult;
    void           *unmatchedmark;
    void           *unregistered;
    void           *VMerror;

    /* typenames */
    void           *integertype;
    void           *realtype;
    void           *booleantype;
    void           *arraytype;
    void           *stringtype;
    void           *nametype;
    void           *dicttype;
    void           *operatortype;
    void           *filetype;
    void           *marktype;
    void           *nulltype;
    void           *savetype;
    void           *fonttype;
   
    /* misc state */
    int             GotInterrupt;    
};


typedef struct _PlincOp PlincOp;
struct _PlincOp {
    void  *(*Func)(PlincInterp *);
    char    *Name;
};


/****************************************/

PlincInterp    *PlincNewInterp(size_t heap);
void            PlincFreeInterp(PlincInterp *i);
void            PlincInitErrorNames(PlincInterp *i);
void            PlincInitTypeNames(PlincInterp *i);
void            PlincInitVals(PlincInterp *i);

void           *PlincNewArray(PlincHeap *h, PlincUInt size);
void           *PlincArrayVal(PlincInterp *i, PlincVal *a, PlincVal *ret);

void           *PlincNewString(PlincHeap *h, PlincUInt size);

void           *PlincNewDict(PlincHeap *h, PlincUInt size);
PlincUInt       PlincHash(PlincUInt r);
PlincUInt       PlincHashVal(PlincVal *v);
void           *PlincPutDictName(PlincInterp *i, PlincDict *d,
                    void *key, PlincVal *val);
void           *PlincPutDict(PlincInterp *i, PlincDict *d,
                    PlincVal *key, PlincVal *val);
void           *PlincGetDict(PlincInterp *i, PlincDict *d,
                    PlincVal *key, PlincVal *val);
void           *PlincLoadDict(PlincInterp *i, PlincVal *key, PlincVal *val);

void            PlincInitOps(PlincInterp *i, const PlincOp *o);
void            PlincInitStackOps(PlincInterp *i);
void            PlincInitTypeOps(PlincInterp *i);
void            PlincInitPrintOps(PlincInterp *i);
void            PlincInitArithOps(PlincInterp *i);
void            PlincInitArrayOps(PlincInterp *i);
void            PlincInitStringOps(PlincInterp *i);
void            PlincInitDictOps(PlincInterp *i);
void            PlincInitRealOps(PlincInterp *i);
void            PlincInitRelationalOps(PlincInterp *i);
void            PlincInitControlOps(PlincInterp *i);
void            PlincInitLoopOps(PlincInterp *i);
void            PlincInitPolymorphOps(PlincInterp *i);
void            PlincInitVMOps(PlincInterp *i);

int             PlincEqual(PlincVal *v1, PlincVal *v2);
void           *PlincReprVal(PlincInterp *i, PlincVal *v);
void            PlincClearN(PlincInterp *i, int n);

void           *PlincGo(PlincInterp *i);
void           *PlincExecStr(PlincInterp *i, const char *s);


#endif /* PLINC_INTERP_H */

/* $Endicor: stack.h,v 1.2 1999/01/14 02:55:42 tsarna Exp tsarna $ */

#include <plinc/types.h>


typedef struct _PlincStack PlincStack;
struct _PlincStack {
    PlincVal       *Stack;
    PlincUInt       Len;
    PlincUInt       MinLen;
    PlincUInt       MaxLen;
};


#define PLINC_INCREF_VAL(v) v
#define PLINC_DECREF_VAL(v) v

#define PLINC_PUSH(s, v)  do { (s).Stack[((s).Len)++] = v; } while (0)
#define PLINC_POP(s)      do { PLINC_DECREF_VAL((s).Stack[--((s).Len)]); } while (0)

#define PLINC_TOPDOWN(s, i)     ((s).Stack[(s).Len - 1 - (i)])

/****************************************/

int     PlincNewStack(PlincStack *s, size_t size);
void    PlincFreeStack(PlincStack *s);

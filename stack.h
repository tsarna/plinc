/* $Endicor: stack.h,v 1.1 1999/01/14 01:26:00 tsarna Exp tsarna $ */

#include <plinc/types.h>


typedef struct _PlincStack PlincStack;
struct _PlincStack {
    PlincVal       *Stack;
    PlincUInt       Len;
    PlincUInt       MinLen;
    PlincUInt       MaxLen;
};


#define PLINC_PUSH(s, v)  do {(s).Stack[++((s).Len)] = v;} while (0)


/****************************************/

int     PlincNewStack(PlincStack *s, size_t size);
void    PlincFreeStack(PlincStack *s);

/* $Endicor$ */

#include <plinc/types.h>


typedef struct _PlincStack PlincStack;
struct _PlincStack {
    PlincVal       *Stack;
    PlincUInt       StackLen;
    PlincUInt       StackMinLen;
    PlincUInt       StackMaxLen;
};

/****************************************/

int     PlincNewStack(PlincStack *s, size_t size);
void    PlincFreeStack(PlincStack *s);

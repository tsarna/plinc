/* $Endicor: stack.h,v 1.3 1999/01/14 05:02:15 tsarna Exp tsarna $ */

#include <plinc/types.h>

typedef struct _PlincStack PlincStack;
struct _PlincStack {
    PlincVal       *Stack;
    PlincUInt       Len;
    PlincUInt       MinLen;
    PlincUInt       MaxLen;
};


#define PLINC_INCREF_VAL(v)     /*v*/
#define PLINC_DECREF_VAL(v)     /*v*/

#define PLINC_PUSH(s, v)        do { (s).Stack[((s).Len)++] = v; } while (0)
#define PLINC_OPPUSH(i, v)      PLINC_PUSH((i)->OpStack, (v))

#define PLINC_POP(s)            do { PLINC_DECREF_VAL((s).Stack[--((s).Len)]); } while (0)
#define PLINC_OPPOP(i)          PLINC_POP((i)->OpStack)

#define PLINC_TOPDOWN(s, n)     ((s).Stack[(s).Len - 1 - (n)])
#define PLINC_OPTOPDOWN(i, n)   PLINC_TOPDOWN((i)->OpStack, (n))

#define PLINC_STACKROOM(s, n)   (((s).Len + (n)) <= (s).MaxLen)
#define PLINC_OPSTACKROOM(i, n) PLINC_STACKROOM((i)->OpStack, (n))

#define PLINC_STACKHAS(s, n)    ((s).Len >= (n))
#define PLINC_OPSTACKHAS(i, n)  PLINC_STACKHAS((i)->OpStack, (n))

/****************************************/

int     PlincNewStack(PlincStack *s, size_t size);
void    PlincFreeStack(PlincStack *s);

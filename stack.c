/* $Endicor$ */

#include <plinc/stack.h>

#include <stdlib.h>


int
PlincNewStack(PlincStack *s, size_t size)
{
    int ok = FALSE;
    
    s->Stack = malloc(sizeof(PlincVal) * size);
    if (s->Stack) {
        ok = TRUE;
        
        s->StackLen = 0;
        s->StackMinLen = 0;
        s->StackMaxLen = size;
    }
    
    return ok;
}



void
PlincFreeStack(PlincStack *s)
{
    if (s && s->Stack) {
        free(s->Stack);
    }
}

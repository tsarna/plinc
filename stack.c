/* $Endicor: stack.c,v 1.1 1999/01/14 01:26:00 tsarna Exp tsarna $ */

#include <plinc/stack.h>

#include <stdlib.h>


int
PlincNewStack(PlincStack *s, size_t size)
{
    int ok = FALSE;
    
    s->Stack = malloc(sizeof(PlincVal) * size);
    if (s->Stack) {
        ok = TRUE;
        
        s->Len = 0;
        s->MinLen = 0;
        s->MaxLen = size;
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

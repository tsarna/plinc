/* $Endicor: stack.c,v 1.3 1999/01/17 21:04:54 tsarna Exp tsarna $ */

#include <plinc/interp.h>

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


static void *
op_pop(PlincInterp *i)
{
    if (PLINC_OPSTACKHAS(i, 1)) {
        PLINC_OPPOP(i);

        return NULL;
    } else {
        return i->stackunderflow;
    }
}


static void *
op_exch(PlincInterp *i)
{
    PlincVal v;
    
    if (PLINC_OPSTACKHAS(i, 1)) {
        v = PLINC_OPTOPDOWN(i, 0);

        PLINC_OPTOPDOWN(i, 0) = PLINC_OPTOPDOWN(i, 1);
        PLINC_OPTOPDOWN(i, 1) = v;
        
        return NULL;
    } else {
        return i->stackunderflow;
    }    
}



static void *
op_dup(PlincInterp *i)
{
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        PLINC_OPPUSH(i, PLINC_OPTOPDOWN(i, 0));
        
        return NULL;
    }
}



static void *
op_index(PlincInterp *i)
{
    PlincVal *v;
    int j;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        if (PLINC_TYPE(*v) != PLINC_TYPE_INT) {
            return i->typecheck;
        } else {
            j = v->Val.Int;
            if (j < 0) {
                return i->rangecheck;
            } else if (!PLINC_OPSTACKHAS(i, j + 1)) {
                return i->stackunderflow;
            } else {
                PLINC_OPTOPDOWN(i, 0) = PLINC_OPTOPDOWN(i, j + 1);
                PLINC_INCREF_VAL(PLINC_OPTOPDOWN(i, 0));
                
                return NULL;
            }
        }
    }
}



static void
clear_n(PlincInterp *i, int n)
{
    int j = n;
    
    while (j) {
        j--;
        
        PLINC_DECREF_VAL(PLINC_OPTOPDOWN(i, j));
    }
    
    i->OpStack.Len -= n;
}



static void *
op_clear(PlincInterp *i)
{
    clear_n(i, i->OpStack.Len);

    return NULL;
}



static void *
op_count(PlincInterp *i)
{
    if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        PlincVal v;
        
        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
        v.Val.Int = i->OpStack.Len;
        
        PLINC_OPPUSH(i, v);
        
        return NULL;
    }
}



static PlincOp ops[] = {
    {"pop",     op_pop},
    {"exch",    op_exch},
    {"dup",     op_dup},

    {"index",   op_index},

    {"clear",   op_clear},
    {"count",   op_count},

    {NULL,      NULL}
};



void
PlincInitStackOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

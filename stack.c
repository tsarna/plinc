/* $Endicor: stack.c,v 1.12 1999/01/25 04:35:58 tsarna Exp $ */

#include <plinc/interp.h>

#include <stdlib.h>
#include <string.h>


static int counttomark(PlincInterp *i);


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
                return i->rangecheck;
            } else {
                PLINC_OPTOPDOWN(i, 0) = PLINC_OPTOPDOWN(i, j + 1);
                PLINC_INCREF_VAL(PLINC_OPTOPDOWN(i, 0));

                return NULL;
            }
        }
    }
}



static void *
op_roll(PlincInterp *i)
{
    PlincVal *v, t;
    int n, r;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        if (PLINC_TYPE(*v) != PLINC_TYPE_INT) {
            return i->typecheck;
        } else {
            r = v->Val.Int;

            v = &PLINC_OPTOPDOWN(i, 1);
            if (PLINC_TYPE(*v) != PLINC_TYPE_INT) {
                return i->typecheck;
            } else {
                n = v->Val.Int;

                if (n < 0) {
                    return i->rangecheck;
                } else if (!PLINC_OPSTACKHAS(i, n + 1)) {
                    return i->rangecheck;
                } else {
                    PLINC_OPPOP(i);
                    PLINC_OPPOP(i);

                    while ((r > 0) && (n > 1)) {
                        t = PLINC_OPTOPDOWN(i, 0);

                        memmove(&PLINC_OPTOPDOWN(i, n - 2),
                                &PLINC_OPTOPDOWN(i, n - 1),
                                sizeof(PlincVal) * (n - 1));

                        PLINC_OPTOPDOWN(i, n - 1) = t;

                        r--;
                    }

                    while ((r < 0) && (n > 1)) {
                        t = PLINC_OPTOPDOWN(i, n - 1);

                        memmove(&PLINC_OPTOPDOWN(i, n - 1),
                                &PLINC_OPTOPDOWN(i, n - 2),
                                sizeof(PlincVal) * (n - 1));

                        PLINC_OPTOPDOWN(i, 0) = t;

                        r++;
                    }

                    return NULL;
                }
            }
        }
    }
}



void
PlincClearN(PlincInterp *i, int n)
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
    PlincClearN(i, i->OpStack.Len);

    return NULL;
}



static void *
countstack(PlincInterp *i, PlincStack *s)
{
    if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        PlincVal v;

        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
        v.Val.Int = s->Len;

        PLINC_OPPUSH(i, v);

        return NULL;
    }
}



static void *
op_count(PlincInterp *i)
{
    return countstack(i, &(i->OpStack));
}



static void *
op_countdictstack(PlincInterp *i)
{
    return countstack(i, &(i->DictStack));
}



static void *
op_countexecstack(PlincInterp *i)
{
    return countstack(i, &(i->ExecStack));
}



static int
counttomark(PlincInterp *i)
{
    int j;

    for (j = 0; j < i->OpStack.Len; j++) {
        if (PLINC_IS_MARK(PLINC_OPTOPDOWN(i, j))) {
            return j;
        }
    }

    return -1;
}



static void *
op_counttomark(PlincInterp *i)
{
    if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        int j;

        j = counttomark(i);
        if (j < 0) {
            return i->unmatchedmark;
        } else {
            PlincVal v;

            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
            v.Val.Int = j;

            PLINC_OPPUSH(i, v);

            return NULL;
        }
    }
}



static void *
op_cleartomark(PlincInterp *i)
{
    int j;

    j = counttomark(i);
    if (j < 0) {
        return i->unmatchedmark;
    } else {
        PlincClearN(i, j+1);

        return NULL;
    }
}



static void *
setflags(PlincInterp *i, PlincUInt set, PlincUInt clear)
{
    PlincUInt32 *f;
    PlincVal *v;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_CAN_WRITE(*v)) {
            f = &(v->Flags);
            if (PLINC_TYPE(*v) == PLINC_TYPE_DICT) {
                f = &(((PlincDict *)(v->Val.Ptr))->Flags);
            }

            *f &= (~clear);
            *f |= set;

            return NULL;
        } else {
            return i->invalidaccess;
        }
    }
}



static void *
op_cvlit(PlincInterp *i)
{
    return setflags(i, PLINC_ATTR_LIT, 0);
}



static void *
op_cvx(PlincInterp *i)
{
    return setflags(i, 0, PLINC_ATTR_LIT);
}



static void *
op_dot_doexec(PlincInterp *i)
{
    return setflags(i, PLINC_ATTR_DOEXEC, PLINC_ATTR_LIT);
}



static void *
op_xcheck(PlincInterp *i)
{
    PlincVal v;
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
        if (PLINC_EXEC(PLINC_OPTOPDOWN(i, 0))) {
            v.Val.Int = 1;
        } else {
            v.Val.Int = 0;
        }
        
        PLINC_OPPOP(i);
        PLINC_OPPUSH(i, v);

        return NULL;
    }
}



static void *
copystack(PlincInterp *i, PlincStack *s)
{
    PlincVal *v;
    int j;
    
    if (!PLINC_OPSTACKHAS(i, 0)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        if (PLINC_TYPE(*v) != PLINC_TYPE_ARRAY) {
            return i->typecheck;
        } else if ((s->Len) > PLINC_SIZE(*v)) {
            return i->rangecheck;
        } else {
            memcpy(v->Val.Ptr, s->Stack, s->Len * sizeof(PlincVal));
            
            for (j = 0; j < s->Len; j++) {
                PLINC_INCREF_VAL(s->Stack[j]);
            }
            
            /* adjust to initial subrange */
            v->Flags &= ~PLINC_SIZE_MASK;
            v->Flags |= s->Len;
            
            return NULL;
        }
    }
}



static void *
op_dictstack(PlincInterp *i)
{
    return copystack(i, &(i->DictStack));
}



static void *
op_execstack(PlincInterp *i)
{
    return copystack(i, &(i->ExecStack));
}



static const PlincOp ops[] = {
    {op_pop,            "pop"},
    {op_exch,           "exch"},
    {op_dup,            "dup"},
    {op_index,          "index"},
    {op_roll,           "roll"},
    {op_clear,          "clear"},
    {op_count,          "count"},
    {op_countdictstack, "countdictstack"},
    {op_countexecstack, "countexecstack"},
    {op_cleartomark,    "cleartomark"},
    {op_counttomark,    "counttomark"},
    {op_dictstack,      "dictstack"},
    {op_execstack,      "execstack"},
    {op_cvlit,          "cvlit"},
    {op_cvx,            "cvx"},
    {op_dot_doexec,     ".doexec"},
    {op_xcheck,         "xcheck"},

    {NULL,          NULL}
};



void
PlincInitStackOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

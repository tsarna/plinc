/* $Endicor: stack.c,v 1.9 1999/01/20 20:30:12 tsarna Exp $ */

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





static const PlincOp ops[] = {
    {"pop",         op_pop},
    {"exch",        op_exch},
    {"dup",         op_dup},

    {"index",       op_index},
    {"roll",        op_roll},
    {"clear",       op_clear},
    {"count",       op_count},
    {"cleartomark", op_cleartomark},
    {"counttomark", op_counttomark},

    {"cvlit",       op_cvlit},
    {"cvx",         op_cvx},
    {".doexec",     op_dot_doexec},
    {"xcheck",      op_xcheck},

    {NULL,          NULL}
};



void
PlincInitStackOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

/* $Endicor: loop.c,v 1.6 1999/01/27 20:15:36 tsarna Exp $ */

#include <plinc/interp.h>
#include <stdlib.h>


/* numbers both distinguish types and say how much to pop when done */

#define LOOP_LOOP       2       /* {block} .looper. */
#define LOOP_REPEAT     3       /* 10 {block} .looper */       
#define LOOP_FORALL     4       /* object ptr {block} .looper */
#define LOOP_FOR        5       /* 0 1 10 {block} .looper */


static const PlincOp ops[];     /* forward */


static void *
op_dot_looper(PlincInterp *i)
{
    PlincVal *l, *n, *x, v;
    PlincDict *d;
    
    l = &PLINC_TOPDOWN(i->ExecStack, 0);

    if (PLINC_SIZE(*l) == LOOP_REPEAT) {
        n = &PLINC_TOPDOWN(i->ExecStack, 2);
        if (n->Val.Int) {
            if (!PLINC_STACKROOM(i->ExecStack, 1)) {
                return i->execstackoverflow;
            } else {
                PLINC_PUSH(i->ExecStack, PLINC_TOPDOWN(i->ExecStack, 1));
                n->Val.Int--;
            }
        } else {
            PLINC_XPOP(i); /* .looper */
            PLINC_XPOP(i); /* proc */
            PLINC_XPOP(i); /* count */
        }
    } else if (PLINC_SIZE(*l) == LOOP_LOOP) {
        if (!PLINC_STACKROOM(i->ExecStack, 1)) {
            return i->execstackoverflow;
        } else {
            PLINC_PUSH(i->ExecStack, PLINC_TOPDOWN(i->ExecStack, 1));
        }
    } else if (PLINC_SIZE(*l) == LOOP_FORALL) {
        if (!PLINC_STACKROOM(i->ExecStack, 1)) {
            return i->execstackoverflow;
        } else {
            n = &PLINC_TOPDOWN(i->ExecStack, 3);
            x = &PLINC_TOPDOWN(i->ExecStack, 2);

            if (PLINC_TYPE(*n) == PLINC_TYPE_STRING) {
                if (x->Val.Int < PLINC_SIZE(*n)) {
                    if (!PLINC_OPSTACKROOM(i, 1)) {
                        return i->stackoverflow;
                    }
                    
                    PLINC_OPPUSH(i, ((PlincVal *)(n->Val.Ptr))[x->Val.Int++]);
                    PLINC_PUSH(i->ExecStack, PLINC_TOPDOWN(i->ExecStack, 1));
                } else {
                    PLINC_XPOP(i); /* .looper */
                    PLINC_XPOP(i); /* proc */
                    PLINC_XPOP(i); /* count */
                    PLINC_XPOP(i); /* object */
                }
            } else if (PLINC_TYPE(*n) == PLINC_TYPE_ARRAY) {
                if (x->Val.Int < PLINC_SIZE(*n)) {
                    if (!PLINC_OPSTACKROOM(i, 1)) {
                        return i->stackoverflow;
                    }
                    
                    v = ((PlincVal *)(n->Val.Ptr))[x->Val.Int++];
                    
                    PLINC_OPPUSH(i, v);
                    PLINC_PUSH(i->ExecStack, PLINC_TOPDOWN(i->ExecStack, 1));
                } else {
                    PLINC_XPOP(i); /* .looper */
                    PLINC_XPOP(i); /* proc */
                    PLINC_XPOP(i); /* count */
                    PLINC_XPOP(i); /* object */
                }
            } else if (PLINC_TYPE(*n) == PLINC_TYPE_DICT) {
                d = n->Val.Ptr;
                
                if (!PLINC_OPSTACKROOM(i, 2)) {
                    return i->stackoverflow;
                }

                while (x->Val.Int < d->MaxLen) {
                    if (PLINC_IS_NULL(d->Vals[x->Val.Int].Key)) {
                        x->Val.Int++;
                    } else {
                        PLINC_OPPUSH(i, d->Vals[x->Val.Int].Key);
                        PLINC_OPPUSH(i, d->Vals[x->Val.Int++].Val);
                        PLINC_PUSH(i->ExecStack, PLINC_TOPDOWN(i->ExecStack, 1));
                    
                        return i;
                    }
                }

                PLINC_XPOP(i); /* .looper */
                PLINC_XPOP(i); /* proc */
                PLINC_XPOP(i); /* count */
                PLINC_XPOP(i); /* object */
            }
        }
    }

    return i;
}



static void *
op_repeat(PlincInterp *i)
{
    PlincVal *v1, *v2, v;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else if (!PLINC_STACKROOM(i->ExecStack, 2)) {
        return i->execstackoverflow;
    } else {
        v2 = &PLINC_OPTOPDOWN(i, 0);
        v1 = &PLINC_OPTOPDOWN(i, 1);

        if ((PLINC_TYPE(*v2) != PLINC_TYPE_ARRAY)
        ||  (PLINC_TYPE(*v1) != PLINC_TYPE_INT)) {
            return i->typecheck;
        } else if (!PLINC_CAN_EXEC(*v2)) {
            return i->invalidaccess;
        } else if (v1->Val.Int < 0) {
            return i->rangecheck;
        } else {
            PLINC_TOPDOWN(i->ExecStack, 0) = *v1;
            PLINC_PUSH(i->ExecStack, *v2);
            PLINC_TOPDOWN(i->ExecStack, 0).Flags |= PLINC_ATTR_DOEXEC;

            v.Flags = PLINC_TYPE_OP | LOOP_REPEAT;
            v.Val.Op = (PlincOp *)(&ops[0]);
            PLINC_PUSH(i->ExecStack, v);

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
        }
    }

    return i;
}



static void *
op_loop(PlincInterp *i)
{
    PlincVal *v1, v;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else if (!PLINC_STACKROOM(i->ExecStack, 1)) {
        return i->execstackoverflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v1) != PLINC_TYPE_ARRAY) {
            return i->typecheck;
        } else if (!PLINC_CAN_EXEC(*v1)) {
            return i->invalidaccess;
        } else {
            PLINC_TOPDOWN(i->ExecStack, 0) = *v1;
            PLINC_TOPDOWN(i->ExecStack, 0).Flags |= PLINC_ATTR_DOEXEC;

            v.Flags = PLINC_TYPE_OP | LOOP_LOOP;
            v.Val.Op = (PlincOp *)(&ops[0]);
            PLINC_PUSH(i->ExecStack, v);

            PLINC_OPPOP(i);
        }
    }

    return i;
}



static void *
op_exit(PlincInterp *i)
{
    PlincVal *v;
    int j;
     
    for (j = 0; j < i->ExecStack.Len; j++) {
        v = &PLINC_TOPDOWN(i->ExecStack, j);
        
        if (PLINC_TYPE(*v) == PLINC_TYPE_OP) {
            if (v->Val.Op == &ops[0]) {
                while (j) {
                    j += PLINC_SIZE(*v);
                    
                    if (!PLINC_STACKHAS(i->ExecStack, j)) {
                        return i->invalidexit;
                    } else {
                        while (j) {
                            PLINC_XPOP(i);
                            j--;
                        }
                        
                        return i;
                    }
                }
            }
        }
    }

    return i->invalidexit;
}



static void *
op_forall(PlincInterp *i)
{
    PlincVal *v1, *v2, v;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else if (!PLINC_STACKROOM(i->ExecStack, 3)) {
        return i->execstackoverflow;
    } else {
        v2 = &PLINC_OPTOPDOWN(i, 0);
        v1 = &PLINC_OPTOPDOWN(i, 1);

        if ((PLINC_TYPE(*v2) != PLINC_TYPE_ARRAY)
        || (
            (PLINC_TYPE(*v1) != PLINC_TYPE_STRING)
         && (PLINC_TYPE(*v1) != PLINC_TYPE_ARRAY)
         && (PLINC_TYPE(*v1) != PLINC_TYPE_DICT)
        )) {
            return i->typecheck;
        } else if (!PLINC_CAN_EXEC(*v2) || !PLINC_CAN_READ(*v1)) {
            return i->invalidaccess;
        } else {
            PLINC_TOPDOWN(i->ExecStack, 0) = *v1;

            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
            v.Val.Int = 0;
            PLINC_PUSH(i->ExecStack, v);

            PLINC_PUSH(i->ExecStack, *v2);
            PLINC_TOPDOWN(i->ExecStack, 0).Flags |= PLINC_ATTR_DOEXEC;

            v.Flags = PLINC_TYPE_OP | LOOP_FORALL;
            v.Val.Op = (PlincOp *)(&ops[0]);
            PLINC_PUSH(i->ExecStack, v);

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
        }
    }

    return i;
}



static const PlincOp ops[] = {
    {op_dot_looper,     ".looper"}, /* must be first */
    {op_repeat,         "repeat"},
    {op_loop,           "loop"},
    {op_exit,           "exit"},
    {op_forall,         "forall"},

    {NULL,              NULL}
};



void
PlincInitLoopOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

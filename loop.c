/* $Endicor: exec.c,v 1.11 1999/01/20 20:30:12 tsarna Exp tsarna $ */

#include <plinc/interp.h>
#include <stdlib.h>


/* numbers both distinguish types and say how much to pop when done */

#define LOOP_LOOP       1
#define LOOP_FORALL     2
#define LOOP_REPEAT     3
#define LOOP_FOR        4


static const PlincOp ops[];     /* forward */


static void *
op_dot_looper(PlincInterp *i)
{
    PlincVal *l, *n;
    
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
            PLINC_POP(i->ExecStack); /* .looper */
            PLINC_POP(i->ExecStack); /* proc */
            PLINC_POP(i->ExecStack); /* count */
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



static const PlincOp ops[] = {
    {".looper",     op_dot_looper}, /* must be first */
    {"repeat",      op_repeat},

    {NULL,          NULL}
};



void
PlincInitLoopOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

#include <plinc/token.h>
#include <stdio.h> /*XXX*/
#include <string.h>


static void *ValFromComp(PlincInterp *i, void *r, PlincVal *v, PlincVal *nv);

static const PlincOp ops[]; /* forward */


void
pres(PlincInterp *i)
{
    int j;

#if 0
    fprintf(stderr, "XXX: Exec Stack len %d\n", i->ExecStack.Len);
#endif
    for (j = 0; j < i->ExecStack.Len; j++) {
#if 0
        fprintf(stderr, "XXX: ");
        PlincReprVal(i, &PLINC_TOPDOWN(i->ExecStack, j));
        fprintf(stderr, "\n");
#endif
    }
}

void *
PlincGo(PlincInterp *i)
{
    PlincVal *v, nv;
    void *r;

    i->GotInterrupt = FALSE;

    while (i->ExecStack.Len) {
        if (i->GotInterrupt) {
            return i->interrupt;
        }

        v = &PLINC_TOPDOWN(i->ExecStack, 0);

pres(i);
        if ((PLINC_EXEC(*v) && !(i->ScanLevel)) || PLINC_DOEXEC(*v)) {
            if (!PLINC_CAN_EXEC(*v)) {
                return i->invalidaccess;
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_OP) {
                r = v->Val.Op->Func(i);
                if (r == i) {
                    continue;
                } else if (r) {
                    fprintf(stderr, "ERR %s\n", (char *)r);
                    return r;
                } else {
                    PLINC_POP(i->ExecStack);
                    continue;
                }
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_NAME) {
                r = PlincLoadDict(i, v, &nv);
                if (r) {
                    return r;
                } else {
                    PLINC_INCREF_VAL(nv);
                    *v = nv;

                    /* force procs to be run rather than pushed */
                    if (((PLINC_TYPE(*v) == PLINC_TYPE_ARRAY)
                    ||  (PLINC_TYPE(*v) == PLINC_TYPE_OP))
                    &&   PLINC_EXEC(*v)) {
                        v->Flags |= PLINC_ATTR_DOEXEC;
                    }

                    continue;
                }
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_ARRAY) {
                r = PlincArrayVal(i, v, &nv);
                r = ValFromComp(i, r, v, &nv);
                if (r) {
                    return r;
                }
                continue;
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_STRING) {
                r = PlincTokenVal(i, v, &nv);
                r = ValFromComp(i, r, v, &nv);
                if (r) {
                    return r;
                }
                continue;
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_FILE) {
                continue;
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_NULL) {
                PLINC_POP(i->ExecStack);

                continue;
            }
        }

        if (!PLINC_OPSTACKROOM(i, 1)) {
            return i->stackoverflow;
        }

        PLINC_OPPUSH(i, *v);
        PLINC_POP(i->ExecStack);
    }

    return NULL;
}



static void *
ValFromComp(PlincInterp *i, void *r, PlincVal *v, PlincVal *nv)
{
    if (r == i) {
        if (!PLINC_SIZE(*v)) {
            /* if we exhausted the string/array, pop it off */
            PLINC_POP(i->ExecStack);
        }

        if ((PLINC_EXEC(*nv) && (i->ScanLevel == 0)
        && (PLINC_TYPE(*nv) != PLINC_TYPE_ARRAY))
        || PLINC_DOEXEC(*nv)) {
            if (!PLINC_STACKROOM(i->ExecStack, 1)) {
                return i->execstackoverflow;
            }

            /* ensure this string continues to run */
            v->Flags |= PLINC_ATTR_DOEXEC;

            PLINC_PUSH(i->ExecStack, *nv);
        } else {
            if (!PLINC_OPSTACKROOM(i, 1)) {
                return i->stackoverflow;
            }

            PLINC_OPPUSH(i, *nv);
        }
    } else if (r) {
        return r;
    } else {
        /* nothing left in string, pop it */
        PLINC_POP(i->ExecStack);
    }

    return NULL;
}



void *
PlincExecStr(PlincInterp *i, const char *s)
{
    PlincVal v;

    v.Flags = PLINC_TYPE_STRING | PLINC_ATTR_NOWRITE | strlen(s);
    v.Val.Ptr = (char *)s;

    PLINC_PUSH(i->ExecStack, v);

    return PlincGo(i);
}



static void *
op_rbrace(PlincInterp *i)
{
    PlincVal v;

    if (PLINC_OPSTACKROOM(i, 1)) {
        i->ScanLevel++;
#if 0
fprintf(stderr, ">>> scanlevel %d\n", i->ScanLevel);
#endif
        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_MARK;
        PLINC_OPPUSH(i, v);

        return NULL;
    } else {
        return i->stackoverflow;
    }
}



static void *
op_dot_decscan(PlincInterp *i)
{
    if (i->ScanLevel) {
        i->ScanLevel--;
#if 0
fprintf(stderr, ">>> scanlevel %d\n", i->ScanLevel);
#endif
        return NULL;
    } else {
        return i->syntaxerror;
    }
}



static void *
op_exec(PlincInterp *i)
{
    PlincVal *v;

    /* replace top of exec stack (which will be the exec operator)
       with the value popped off the operator stack */

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);

        PLINC_TOPDOWN(i->ExecStack, 0) = *v;
        PLINC_OPPOP(i);

        /* tell main loop not to pop the execstack */
        return i;
    }
}



static void *
op_stop(PlincInterp *i)
{
    PlincVal *v, t;
    int j;
     
    for (j = 0; j < i->ExecStack.Len; j++) {
        v = &PLINC_TOPDOWN(i->ExecStack, j);
        
        if (PLINC_TYPE(*v) == PLINC_TYPE_OP) {
            if (v->Val.Op == &ops[0]) {
                while (j) {
                    while (j) {
                        PLINC_POP(i->ExecStack);
                        j--;
                    }
                        
                    if (!PLINC_OPSTACKROOM(i, 1)) {
                        return i->stackoverflow;
                    } else {
                        t.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
                        t.Val.Int = TRUE;
                        PLINC_OPPUSH(i, t);

                        return NULL;
                    }
                }
            }
        }
    }

    return i->invalidstop;
}



static void *
op_stopped(PlincInterp *i)
{
    PlincVal *v, o;

    /* replace top of exec stack (which will be the exec operator)
       with the value popped off the operator stack */

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else if (!PLINC_STACKROOM(i->ExecStack, 1)) {
        return i->execstackoverflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);

        /* force proc to be executed */
        v->Flags |= PLINC_ATTR_DOEXEC;

        o.Flags = PLINC_TYPE_OP;
        o.Val.Ptr = (void *)&ops[0];    /*XXX*/

        PLINC_TOPDOWN(i->ExecStack, 0) = o;
        PLINC_PUSH(i->ExecStack, *v);
        PLINC_OPPOP(i);

        /* tell main loop not to pop the opstack */
        return i;
    }
}



static void *
op_dot_stopped(PlincInterp *i)
{
    PlincVal v;

    if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
        v.Val.Int = FALSE;

        PLINC_OPPUSH(i, v);

        return NULL;
    }
}



static void *
op_if(PlincInterp *i)
{
    PlincVal *v1, *v2;
    void *r = NULL;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);

        if ((PLINC_TYPE(*v1) != PLINC_TYPE_BOOL)
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_ARRAY)) {
            return i->typecheck;
        } else if (!PLINC_CAN_EXEC(*v2)) {
            return i->invalidaccess;
        } else {
            if (v1->Val.Int) {
                PLINC_TOPDOWN(i->ExecStack, 0) = *v2;

                r = i;
            }

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
        }
    }

    return r;
}



static void *
op_ifelse(PlincInterp *i)
{
    PlincVal *v1, *v2, *v3;

    if (!PLINC_OPSTACKHAS(i, 3)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 2);
        v2 = &PLINC_OPTOPDOWN(i, 1);
        v3 = &PLINC_OPTOPDOWN(i, 0);

        if ((PLINC_TYPE(*v1) != PLINC_TYPE_BOOL)
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_ARRAY)
        ||  (PLINC_TYPE(*v3) != PLINC_TYPE_ARRAY)) {
            return i->typecheck;
        } else if (!PLINC_CAN_EXEC(*v2) || !PLINC_CAN_EXEC(*v3)) {
            return i->invalidaccess;
        } else {
            if (v1->Val.Int) {
                PLINC_TOPDOWN(i->ExecStack, 0) = *v2;
            } else {
                PLINC_TOPDOWN(i->ExecStack, 0) = *v3;
            }

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPOP(i);

            return i;
        }
    }
}



static const PlincOp ops[] = {
    {op_dot_stopped,    ".stopped"},    /* XXX must be first */
    {op_rbrace,         "{"},
    {op_dot_decscan,    ".decscan"},
    {op_exec,           "exec"},
    {op_if,             "if"},
    {op_ifelse,         "ifelse"},
    {op_stop,           "stop"},
    {op_stopped,        "stopped"},

    {NULL,              NULL}
};



void
PlincInitControlOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}


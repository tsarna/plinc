#include <plinc/token.h>
#include <plinc/file.h>

#include <stdio.h> /*XXX*/
#include <string.h>


static void *ValFromComp(PlincInterp *i, void *r, PlincVal *v, PlincVal *nv);

static const PlincOp ops[]; /* forward */


void
pres(PlincInterp *i)
{
#if 0
    int j;

    fprintf(stderr, "XXX: Exec Stack len %d\n", i->ExecStack.Len);

    for (j = 0; j < i->ExecStack.Len; j++) {
        fprintf(stderr, "XXX: ");
        PlincReprVal(i, &PLINC_TOPDOWN(i->ExecStack, j));
        fprintf(stderr, "\n");
    }
#endif
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
                    PLINC_XPOP(i);
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
                PLINC_XPOP(i);

                continue;
            }
        }

        if (!PLINC_OPSTACKROOM(i, 1)) {
            return i->stackoverflow;
        }

        PLINC_OPPUSH(i, *v);
        PLINC_XPOP(i);
    }

    return NULL;
}



static void *
ValFromComp(PlincInterp *i, void *r, PlincVal *v, PlincVal *nv)
{
    if (r == i) {
        if (!PLINC_SIZE(*v)) {
            /* if we exhausted the string/array, pop it off */
            PLINC_XPOP(i);
        }

        if ((PLINC_EXEC(*nv) && (i->ScanLevel == 0)
        && (PLINC_TYPE(*nv) != PLINC_TYPE_ARRAY))
        || PLINC_DOEXEC(*nv)) {
            if (!PLINC_XSTACKROOM(i, 1)) {
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
        PLINC_XPOP(i);
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

#if 1
    return PlincExec(i);
#else
    return PlincGo(i);
#endif
}



/***@@@*************************************/



static void *
PushVal(PlincInterp *i, PlincVal *v)
{
    if (PLINC_IS_ARRAY(*v) || PLINC_LIT(*v)) {
        if (PLINC_OPSTACKROOM(i, 1)) {
            PLINC_OPPUSH(i, *v);
            return NULL;
        } else {
            return i->stackoverflow;
        }
    } else {
        if (PLINC_XSTACKROOM(i, 1)) {
            PLINC_PUSH(i->ExecStack, *v);
            return NULL;
        } else {
            return i->execstackoverflow;
        }
    }
}



void *
PlincExec(PlincInterp *i)
{
    void *r = NULL;
    int depth = 0;
    PlincVal *v, nv;
    PlincStrFile sf;
    
    depth = i->ExecStack.Len;

    while (i->ExecStack.Len >= depth) {
        v = &PLINC_TOPDOWN(i->ExecStack, 0);

pres(i);

        switch (v->Flags & (PLINC_TYPE_MASK|PLINC_ATTR_LIT|PLINC_ATTR_NOEXEC)) {
        case PLINC_TYPE_STRING:
            PlincInitStrFile(&sf, v);
            r = PlincGetToken(i, (PlincFile *)(void *)(&sf), &nv);
            v->Val.Ptr = (char *)(v->Val.Ptr) + sf.Cur;
            PLINC_SET_SIZE(*v, PLINC_SIZE(*v) - sf.Cur);
            if (r == i) {
                r = NULL;
            } else if (!r) {
                r = PushVal(i, &nv);
                if (!r) {
                    continue;
                }
            }
            break;


        case PLINC_TYPE_FILE:
            r = PlincGetToken(i, (PlincFile *)(v->Val.Ptr), &nv);
            if (r == i) {
                r = NULL;
            } else if (!r) {
                r = PushVal(i, &nv);
                if (!r) {
                    continue;
                }
            }
            break;


        case PLINC_TYPE_ARRAY:
            if (PLINC_SIZE(*v)) {
                nv = *(PlincVal *)(v->Val.Ptr);
                PLINC_INCREF_VAL(nv);

                v->Val.Ptr = ((PlincVal *)(v->Val.Ptr)) + 1;
                PLINC_SET_SIZE(*v, PLINC_SIZE(*v) - 1);

                if (!PLINC_SIZE(*v)) {
                    PLINC_XPOP(i);
                }
                r = PushVal(i, &nv);
                if (!r) {
                    continue;
                }
            }
            break;

	
        case PLINC_TYPE_NAME:
            r = PlincLoadDict(i, v, &nv);
            if (!r) {
                PLINC_INCREF_VAL(nv);
                *v = nv;
                continue;
            }
            break;
            

	case PLINC_TYPE_OP:
	    r = v->Val.Op->Func(i);
	    if (r == i) {
	        r = NULL;
	        continue;
	    }
	    break;
            

        case PLINC_TYPE_STRING|PLINC_ATTR_NOEXEC:
        case PLINC_TYPE_FILE|PLINC_ATTR_NOEXEC:
        case PLINC_TYPE_ARRAY|PLINC_ATTR_NOEXEC:
            r = i->invalidaccess;
            break;


        default:
            if (PLINC_OPSTACKROOM(i, 1)) {
                PLINC_OPPUSH(i, *v);
            } else {
                r = i->stackoverflow;
            }

            break;
        }

    error:
        if (r) {
            if (r == i->stackoverflow) {
                nv.Val.Ptr = PlincNewArray(i->Heap, i->OpStack.Len);
                if (nv.Val.Ptr) {
                    nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_ARRAY | (i->OpStack.Len);

                    /* XXX ARRAYPUT */
                    memcpy(nv.Val.Ptr, i->OpStack.Stack,
                        sizeof(PlincVal) * (i->OpStack.Len));
                    i->OpStack.Len = 1;
                    *(i->OpStack.Stack) = nv;
                } else {
                    while (i->OpStack.Len) {
                        PLINC_OPPOP(i);
                    }
                    r = i->VMerror;
                }
            }
            
            nv.Flags = PLINC_TYPE_NAME;
            nv.Val.Ptr = r;
            r = PlincGetDict(i, i->errordict, &nv, &nv);
            if (r == i->undefined) {
                return nv.Val.Ptr; /* XXX */
            } else if (!r) {
                if (PLINC_OPSTACKROOM(i, 1)) {
                    PLINC_OPPUSH(i, *v);
                    PLINC_XPOP(i);
                    PLINC_XPUSH(i, nv);
                    r = NULL;
                    continue;
                } else {
                    r = i->stackoverflow;
                }
            }

            goto error;
        }

        PLINC_XPOP(i);
    }
    
    return NULL;
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
        i->OpStack.Len--;

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
                        PLINC_XPOP(i);
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
    } else if (!PLINC_XSTACKROOM(i, 1)) {
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


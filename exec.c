/* $Endicor: exec.c,v 1.6 1999/01/18 05:13:40 tsarna Exp tsarna $ */

#include <plinc/token.h>
#include <stdio.h> /*XXX*/


static void *ValFromComp(PlincInterp *i, void *r, PlincVal *v, PlincVal *nv);


void
pres(PlincInterp *i)
{
    int j;
    
    fprintf(stderr, "XXX: Exec Stack len %d\n", i->ExecStack.Len);
    for (j = 0; j < i->ExecStack.Len; j++) {
        fprintf(stderr, "XXX: ");
        PlincReprVal(i, &PLINC_TOPDOWN(i->ExecStack, j));
        fprintf(stderr, "\n");
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
                    v->Flags |= PLINC_ATTR_DOEXEC;
                    
                    continue;
                }
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_ARRAY) {
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
        if (PLINC_EXEC(*nv) 
        && (PLINC_DOEXEC(*v) || (PLINC_TYPE(*v) != PLINC_TYPE_ARRAY))) {
            if (!PLINC_STACKROOM(i->ExecStack, 1)) {
                return i->execstackoverflow;
            }
                    
            PLINC_PUSH(i->ExecStack, *nv);
        } else {
            if (!PLINC_OPSTACKROOM(i, 1)) {
                return i->stackoverflow;
            }
        
            PLINC_OPPUSH(i, *nv);
        }        

        if (!PLINC_SIZE(*v)) {
            /* if we exhausted the string/array, pop it off */ 
            PLINC_POP(i->ExecStack);
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
PlincExecStr(PlincInterp *i, char *s)
{
    PlincVal v;
    
    v.Flags = PLINC_TYPE_STRING | strlen(s);
    v.Val.Ptr = s;

    PLINC_PUSH(i->ExecStack, v);
    
    return PlincGo(i);
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

        /* force proc to be executed */
        v->Flags |= PLINC_ATTR_DOEXEC;

        PLINC_INCREF_VAL(*v);
        PLINC_TOPDOWN(i->ExecStack, 0) = *v;
        PLINC_OPPOP(i);

        /* tell main loop not to pop the opstack */
        return i;
    }
}



static void *
op_if(PlincInterp *i)
{
    PlincVal *v1, *v2;
    
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 0);
        v2 = &PLINC_OPTOPDOWN(i, 1);

        if ((PLINC_TYPE(*v2) != PLINC_TYPE_BOOL)
        ||  (PLINC_TYPE(*v1) != PLINC_TYPE_ARRAY)) {
            return i->typecheck;
        } else if (!PLINC_CAN_EXEC(*v1)) {
            return i->invalidaccess;
        } else {
            PLINC_OPPOP(i);
            PLINC_OPPOP(i);

            if (v2->Val.Int) {
                PLINC_INCREF_VAL(*v2);
                PLINC_TOPDOWN(i->ExecStack, 0) = *v2;
                
                return i;
            }
        }
    }
    
    return NULL;
}


static PlincOp ops[] = {
    {"exec",        op_exec},
    {"if",          op_if},

    {NULL,          NULL}
};



void
PlincInitControlOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

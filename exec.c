/* $Endicor: exec.c,v 1.3 1999/01/17 05:14:05 tsarna Exp tsarna $ */

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
    
    while (i->ExecStack.Len) {
        v = &PLINC_TOPDOWN(i->ExecStack, 0);

pres(i);
        if ((PLINC_EXEC(*v) && !(i->ScanLevel)) || PLINC_DOEXEC(*v)) {
            if (!PLINC_CAN_EXEC(*v)) {
                return i->invalidaccess;
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_OP) {
                r = v->Val.Op->Func(i);
                if (r) {
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
        if ((PLINC_TYPE(*nv) == PLINC_TYPE_ARRAY)
        || !PLINC_EXEC(*nv)) {
            if (!PLINC_OPSTACKROOM(i, 1)) {
                return i->stackoverflow;
            }
        
            PLINC_OPPUSH(i, *nv);
        } else {
            if (!PLINC_STACKROOM(i->ExecStack, 1)) {
                return i->execstackoverflow;
            }
                    
            PLINC_PUSH(i->ExecStack, *nv);
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

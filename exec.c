/* $Endicor: exec.c,v 1.1 1999/01/17 02:11:46 tsarna Exp tsarna $ */

#include <plinc/token.h>
#include <stdio.h> /*XXX*/


void
pres(PlincInterp *i)
{
    int j;
    
    fprintf(stderr, "Exec Stack len %d\n", i->ExecStack.Len);
    for (j = 0; j < i->ExecStack.Len; j++) {
        if (PLINC_TYPE(PLINC_TOPDOWN(i->ExecStack, j)) == PLINC_TYPE_STRING) {
            fprintf(stderr, "(%s)/%d\n",
             PLINC_TOPDOWN(i->ExecStack, j).Val.Ptr,
             PLINC_SIZE(PLINC_TOPDOWN(i->ExecStack, j)));
        } else if (PLINC_TYPE(PLINC_TOPDOWN(i->ExecStack, j)) == PLINC_TYPE_INT) {
            fprintf(stderr, "%d\n", PLINC_TOPDOWN(i->ExecStack, j).Val.Int);
        }
    }
    fprintf(stderr, "\n");
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
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_NAME) {
                continue;
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_ARRAY) {
                continue;
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_STRING) {
                r = PlincTokenVal(i, v, &nv);
                if (r == i) {
                    if (!PLINC_STACKROOM(i->ExecStack, 1)) {
                        return i->execstackoverflow;
                    }
                    
                    PLINC_PUSH(i->ExecStack, nv);
                    
                    if (!PLINC_SIZE(*v)) {
                        /* if we exhausted the string, pop it off */ 
                        PLINC_POP(i->ExecStack);
                    }
                    continue;
                } else if (r) {
                    return r;
                } else {
                    /* nothing left in string, pop it */
                    PLINC_POP(i->ExecStack);

                    continue;
                }
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_FILE) {
                continue;
            }
        }
        
        if (!PLINC_OPSTACKROOM(i, 1)) {
            return i->stackoverflow;
        }
fprintf(stderr, "pushing\n");
        
        PLINC_OPPUSH(i, *v);
        PLINC_POP(i->ExecStack);
    }
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

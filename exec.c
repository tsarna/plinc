/* $Endicor$ */

#include <plinc/token.h>


void *
PlincGo(PlincInterp *i)
{
    PlincVal *v, nv;
    void *r;
    
    while (i->ExecStack.Len) {
        v = &PLINC_TOPDOWN(i->ExecStack, 0);

        if ((PLINC_EXEC(v) && !(i->ScanLevel)) || PLINC_DOEXEC(v)) {
            if (!PLINC_CAN_EXEC(v)) {
                return i->invalidaccess;
            } else if (PLINC_TYPE(v) == PLINC_TYPE_NAME) {
                continue;
            } else if (PLINC_TYPE(v) == PLINC_TYPE_ARRAY) {
                continue;
            } else if (PLINC_TYPE(v) == PLINC_TYPE_STRING) {
                r = PlincTokenVal(i, v, &nv);
                if (r == i) {
                    if (!PLINC_STACKROOM(i->ExecStack, 1)) {
                        return i->execstackoverflow;
                    }
                    
                    PLINC_PUSH(i->ExecStack, nv);
                    
                    if (!PLINC_SIZE(v)) {
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
            } else if (PLINC_TYPE(v) == PLINC_TYPE_FILE) {
                continue;
            }
        }
        
        /* push */               
    }
}

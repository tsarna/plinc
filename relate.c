/* $Endicor$ */

#include <plinc/interp.h>
#include <stdio.h> /*XXX*/


int
PlincEqual(PlincVal *v1, PlincVal *v2)
{
    char *p1 = NULL, *p2 = NULL;
    size_t l1 = 0, l2 = 0;
    int t1, t2, eq = FALSE;
    
    t1 = PLINC_TYPE(*v1);
    t2 = PLINC_TYPE(*v2);
    eq = (t1 == t2);
    
    switch (t1) {
    case PLINC_TYPE_BOOL:
        eq = (eq && (v1->Val.Int == v2->Val.Int));
        break;

    case PLINC_TYPE_ARRAY:
    case PLINC_TYPE_DICT:
    case PLINC_TYPE_OP:
    case PLINC_TYPE_FILE:
    case PLINC_TYPE_SAVE:
    case PLINC_TYPE_FONTID:
        eq = (eq && (v1->Val.Ptr == v2->Val.Ptr));
        break;

    case PLINC_TYPE_NAME:
        /* optimization: two names are always equal if their ptr is equal */
        
        if (eq && (v1->Val.Ptr == v2->Val.Ptr)) {
            return TRUE;
        }
        /* FALLTHRU */

    case PLINC_TYPE_STRING:
        eq = FALSE;

        if (t1 == PLINC_TYPE_STRING) {
            p1 = v1->Val.Ptr;
            l1 = PLINC_SIZE(*v1);
        } else {
            p1 = ((char *)(v1->Val.Ptr)) + 1;
            l1 = *(unsigned char *)(v1->Val.Ptr);
        }        

        if (t2 == PLINC_TYPE_STRING) {
            p2 = v2->Val.Ptr;
            l2 = PLINC_SIZE(*v2);
        } else if (t2 == PLINC_TYPE_NAME) {
            p2 = ((char *)(v2->Val.Ptr)) + 1;
            l2 = *(unsigned char *)(v2->Val.Ptr);
        } else {
            return FALSE;
        }
        
        if (l1 == l2) {
            eq = !memcmp(p1, p2, l1);
        }
        break;

    case PLINC_TYPE_INT:
        if (t1 == t2) {
            eq = (v1->Val.Int == v2->Val.Int);
        } else if (t2 == PLINC_TYPE_REAL) {
            eq = ((float)v1->Val.Int == v2->Val.Real);
        } else {
            eq = FALSE;
        }
        break;

    case PLINC_TYPE_REAL:
        if (t1 == t2) {
            eq = (v1->Val.Real == v2->Val.Real);
        } else if (t2 == PLINC_TYPE_INT) {
            eq = (v1->Val.Real == (float)v2->Val.Int);
        } else {
            eq = FALSE;
        }
        break;
    }

    return eq;
}

/* $Endicor: relate.c,v 1.1 1999/01/17 21:04:54 tsarna Exp $ */

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
        eq = (eq &&
             ((v1->Val.Int && v2->Val.Int)
             || !(v1->Val.Int || v2->Val.Int)));
            
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
        /* XXX invalidaccess on strings */
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



static void *
testeq(PlincInterp *i, int invert)
{
    PlincVal *v1, *v2, v;
    int j = 0;
        
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 0);
        v2 = &PLINC_OPTOPDOWN(i, 1);
        
        j = PlincEqual(v1, v2);
        if (invert) {
            j = !j;
        }

        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
        v.Val.Int = j;

        PLINC_OPPOP(i);
        PLINC_OPPOP(i);

        PLINC_OPPUSH(i, v);
        
        return NULL;
    }
}


static void *
op_eq(PlincInterp *i)
{
    return testeq(i, 0);
}



static void *
op_ne(PlincInterp *i)
{
    return testeq(i, 1);
}



static PlincOp ops[] = {
    {"eq",          op_eq},
    {"ne",          op_ne},

    {NULL,          NULL}
};



void
PlincInitRelationalOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}


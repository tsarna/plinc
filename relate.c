#include <plinc/interp.h>
#include <string.h>


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



static void *
op_not(PlincInterp *i)
{
    PlincVal *v;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) == PLINC_TYPE_BOOL) {
            v->Val.Int = !(v->Val.Int);
        } else if (PLINC_TYPE(*v) == PLINC_TYPE_INT) {
            v->Val.Int = ~(v->Val.Int);
        } else {
            return i->typecheck;
        }
        
        return NULL;
    }
}



#define OP_GE  0
#define OP_GT  1
#define OP_LE  2
#define OP_LT  3



int
memrelate(unsigned char *s1, size_t l1, unsigned char *s2, size_t l2)
{
    int r;
    
    r = memcmp(s1, s2, min(l1, l2));
    if (!r) {
        r = l1 - l2;
    }

    r = (r < 0) ? 0 : ((r > 0) ? 2 : 1);
    
    return r;
}



static void *
relate(PlincInterp *i, int op)
{
    PlincVal *v0, *v1, v;
    PlincUInt32 t0, t1;
    int r;
#ifdef WITH_REAL
    float r0, r1;
#endif
    
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v0 = &PLINC_OPTOPDOWN(i, 1);
        v1 = &PLINC_OPTOPDOWN(i, 0);

        t0 = PLINC_TYPE(*v0);
        t1 = PLINC_TYPE(*v1);
        
        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;

        if ((t0 == PLINC_TYPE_INT) && (t1 == PLINC_TYPE_INT)) {
            switch (op) {
            case OP_GE:
                v.Val.Int = ((v0->Val.Int) >= (v1->Val.Int));
                break;
            case OP_GT:
                v.Val.Int = ((v0->Val.Int) > (v1->Val.Int));
                break;
            case OP_LE:
                v.Val.Int = ((v0->Val.Int) <= (v1->Val.Int));
                break;
            case OP_LT:
                v.Val.Int = ((v0->Val.Int) < (v1->Val.Int));
                break;
            }
#ifdef WITH_REAL
        } else if (((t0 == PLINC_TYPE_INT) || (t0 == PLINC_TYPE_REAL))
        &&  ((t1 == PLINC_TYPE_INT) || (t1 == PLINC_TYPE_REAL))) {
            if (t0 == PLINC_TYPE_INT) {
                r0 = (float)(v0->Val.Int);
            } else {
                r0 = v0->Val.Real;
            }

            if (t1 == PLINC_TYPE_INT) {
                r1 = (float)(v1->Val.Int);
            } else {
                r1 = v1->Val.Real;
            }

            switch (op) {
            case OP_GE:
                v.Val.Int = (r0 >= r1);
                break;
            case OP_GT:
                v.Val.Int = (r0 > r1);
                break;
            case OP_LE:
                v.Val.Int = (r0 <= r1);
                break;
            case OP_LT:
                v.Val.Int = (r0 < r1);
                break;
            }
#endif
        } else if ((t0 == PLINC_TYPE_STRING) && (t1 == PLINC_TYPE_STRING)) {
            r = memrelate(v0->Val.Ptr, PLINC_SIZE(*v0),
                          v1->Val.Ptr, PLINC_SIZE(*v1));
            
            /*
             * This nonsense indexes a table as follows:
             *
             *  r is 0, 1, or 2 if the v0 is <, ==, or > than v1.
             *  For each operator, the table indicates which of
             *  <, ==, and > satisfy the operator.
             *  
             *  If r is:   <  ==  >
             *    GE is:   0   1  1
             *    GT is:   0   0  1
             *    LE is:   1   1  0
             *    LT is:   1   0  0
             */

            v.Val.Int = "\0\1\1\0\0\1\1\1\0\1\0\0"[op * 3 + r];
        } else {
            return i->typecheck;
        }

        PLINC_OPPOP(i);
        PLINC_OPPOP(i);
        PLINC_OPPUSH(i, v);
    }

    return NULL;
}



static void *
op_ge(PlincInterp *i)
{
    return relate(i, OP_GE);
}



static void *
op_gt(PlincInterp *i)
{
    return relate(i, OP_GT);
}



static void *
op_le(PlincInterp *i)
{
    return relate(i, OP_LE);
}



static void *
op_lt(PlincInterp *i)
{
    return relate(i, OP_LT);
}



#define OP_AND  0
#define OP_OR   1
#define OP_XOR  2



static void *
andorxor(PlincInterp *i, int op)
{
    PlincVal *v0, *v1, v;
    
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v0 = &PLINC_OPTOPDOWN(i, 1);
        v1 = &PLINC_OPTOPDOWN(i, 0);

        if ((PLINC_TYPE(*v0) == PLINC_TYPE_INT)
        &&  (PLINC_TYPE(*v1) == PLINC_TYPE_INT)) {
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;

            switch (op) {
            case OP_AND:
                v.Val.Int = (v0->Val.Int) & (v1->Val.Int);
                break;
            case OP_OR:
                v.Val.Int = (v0->Val.Int) | (v1->Val.Int);
                break;
            case OP_XOR:
                v.Val.Int = (v0->Val.Int) ^ (v1->Val.Int);
                break;
            }

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);
        } else if ((PLINC_TYPE(*v0) == PLINC_TYPE_BOOL)
        &&  (PLINC_TYPE(*v1) == PLINC_TYPE_BOOL)) {
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;

            switch (op) {
            case OP_AND:
                v.Val.Int = (v0->Val.Int) && (v1->Val.Int);
                break;
            case OP_OR:
                v.Val.Int = (v0->Val.Int) || (v1->Val.Int);
                break;
            case OP_XOR:
                v.Val.Int = (v0->Val.Int != 0) != (v1->Val.Int != 0);
                break;
            }

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);
        } else {
            return i->typecheck;
        }
    }

    return NULL;
}



static void *
op_and(PlincInterp *i)
{
    return andorxor(i, OP_AND);
}



static void *
op_or(PlincInterp *i)
{
    return andorxor(i, OP_OR);
}



static void *
op_xor(PlincInterp *i)
{
    return andorxor(i, OP_XOR);
}



static void *
op_bitshift(PlincInterp *i)
{
    PlincVal *v0, *v1, v;
    int j;
    
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v0 = &PLINC_OPTOPDOWN(i, 1);
        v1 = &PLINC_OPTOPDOWN(i, 0);

        if ((PLINC_TYPE(*v0) != PLINC_TYPE_INT)
        ||  (PLINC_TYPE(*v1) != PLINC_TYPE_INT)) {
            return i->typecheck;
        } else {
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
            
            j = v1->Val.Int;
            if (j > 0) {
                v.Val.Int = (v0->Val.Int) << j;
            } else if (j < 0) {
                v.Val.Int = (v0->Val.Int) >> (-j);
            } else {
                v.Val.Int = v0->Val.Int;
            }

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);

            return NULL;
        }
    }
}



static const PlincOp ops[] = {
    {op_eq,         "eq"},
    {op_ne,         "ne"},
    {op_ge,         "ge"},
    {op_gt,         "gt"},
    {op_le,         "le"},
    {op_lt,         "lt"},
    {op_and,        "and"},
    {op_not,        "not"},
    {op_or,         "or"},
    {op_xor,        "xor"},
    {op_bitshift,   "bitshift"},

    {NULL,      NULL}
};



void
PlincInitRelationalOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

/* $Endicor: arith.c,v 1.9 1999/01/26 04:27:04 tsarna Exp $ */

#include <plinc/interp.h>

#include <stdlib.h>

#ifdef WITH_REAL
#include <math.h>
#include <plinc/fpmath.h>
#endif


static void *
idiv_mod(PlincInterp *i, int op)
{
    PlincVal *v1, *v2, v;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 0);
        v2 = &PLINC_OPTOPDOWN(i, 1);

        if ((PLINC_TYPE(*v1) != PLINC_TYPE_INT)
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_INT)) {
            return i->typecheck;
        } else if (v1->Val.Int == 0) {
            return i->undefinedresult;
        } else {
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
            if (op) {
                v.Val.Int = v2->Val.Int / v1->Val.Int;
            } else {
                v.Val.Int = v2->Val.Int % v1->Val.Int;
            }

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);

            return NULL;
        }
    }
}



static void *
op_idiv(PlincInterp *i)
{
    return idiv_mod(i, TRUE);
}



static void *
op_mod(PlincInterp *i)
{
    return idiv_mod(i, FALSE);
}



static void *
addsub(PlincInterp *i, int add)
{
    PlincVal v1, v2, v;
    int t1, t2;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = PLINC_OPTOPDOWN(i, 1);
        v2 = PLINC_OPTOPDOWN(i, 0);

        t1 = PLINC_TYPE(v1);
        t2 = PLINC_TYPE(v2);

        if ((t1 == PLINC_TYPE_INT) && (t2 == PLINC_TYPE_INT)) {
            int x, y, a, b;
            
            a = v1.Val.Int;
            b = v2.Val.Int;
            x = a + b;
            y = a - b;
            
            if ((add && ((x^a) < 0 && (x^b) < 0))
            ||  (!add && ((y^a) < 0 && (y^~b) < 0))) {
#ifdef WITH_REAL
                /* it would overflow, so convert to reals */

                v1.Val.Real = (float)(v1.Val.Int);
                v2.Val.Real = (float)(v2.Val.Int);

                t1 = t2 = PLINC_TYPE_REAL;
#else
                return i->rangecheck;
#endif
            } else {
                v.Flags = PLINC_TYPE_INT;
                v.Val.Int = add ? x : y;

                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, v);
                
                return NULL;
            }
        }
        
#ifdef WITH_REAL
        if ((t1 == PLINC_TYPE_INT) && (t2 == PLINC_TYPE_REAL)) {
                v1.Val.Real = (float)(v1.Val.Int);

                t1 = PLINC_TYPE_REAL;
        }

        if ((t1 == PLINC_TYPE_REAL) && (t2 == PLINC_TYPE_INT)) {
                v2.Val.Real = (float)(v2.Val.Int);

                t2 = PLINC_TYPE_REAL;
        }

        if ((t1 == PLINC_TYPE_REAL) && (t2 == PLINC_TYPE_REAL)) {
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_REAL;

            PLINC_BEGINFP();
            
            if (add) {
                v.Val.Real = v1.Val.Real + v2.Val.Real;
            } else {
                v.Val.Real = v1.Val.Real - v2.Val.Real;
            }

            if (PLINC_ENDFP(v.Val.Real)) {
                return i->rangecheck;
            } else {
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, v);
            
                return NULL;
            }
        }
#endif

        return i->typecheck;
    }
}



static void *
op_add(PlincInterp *i)
{
    return addsub(i, TRUE);
}



static void *
op_sub(PlincInterp *i)
{
    return addsub(i, FALSE);
}



static void *
op_abs(PlincInterp *i)
{
    PlincVal *v;
#ifdef WITH_REAL
    float x;
#endif

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v) == PLINC_TYPE_INT) {
            if (v->Val.Int < 0) {
                v->Val.Int = -(v->Val.Int);
            }
#ifdef WITH_REAL
        } else if (PLINC_TYPE(*v) == PLINC_TYPE_REAL) {
            PLINC_BEGINFP();
            
            x = fabsf(v->Val.Real);
            
            if (PLINC_ENDFP(v->Val.Real)) {
                return i->rangecheck;
            } else {
                v->Val.Real = x;
            }
#endif
        } else {
            return i->typecheck;
        }
    }

    return NULL;
}



static void *
op_neg(PlincInterp *i)
{
    PlincVal *v;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v) == PLINC_TYPE_INT) {
            v->Val.Int = -(v->Val.Int);
#ifdef WITH_REAL
        } else if (PLINC_TYPE(*v) == PLINC_TYPE_REAL) {
            float x;
            
            PLINC_BEGINFP();
            
            x = -(v->Val.Real);
            
            if (PLINC_ENDFP(x)) {
                return i->rangecheck;
            } else {
                v->Val.Real = x;
            }
#endif
        } else {
            return i->typecheck;
        }
    }

    return NULL;
}



static void *
op_rand(PlincInterp *i)
{
    PlincVal v;
    
    if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
        v.Val.Int = rand_r(&(i->Seed));
        
        PLINC_OPPUSH(i, v);
        
        return NULL;
    }
}



static void *
op_srand(PlincInterp *i)
{
    PlincVal *v;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        if (PLINC_TYPE(*v) != PLINC_TYPE_INT) {
            return i->typecheck;
        } else {
            i->State = i->Seed = v->Val.Int;
            
            PLINC_OPPOP(i);
            
            return NULL;
        }
    }
}



static void *
op_rrand(PlincInterp *i)
{
    PlincVal v;
    
    if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
        v.Val.Int = i->State;
        
        PLINC_OPPUSH(i, v);
        
        return NULL;
    }
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
    {op_add,        "add"},
    {op_idiv,       "idiv"},
    {op_mod,        "mod"},

    {op_sub,        "sub"},
    {op_abs,        "abs"},
    {op_neg,        "neg"},
    {op_rand,       "rand"},
    {op_srand,      "srand"},
    {op_rrand,      "rrand"},
    {op_bitshift,   "bitshift"},

    {NULL,          NULL}
};



void
PlincInitArithOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

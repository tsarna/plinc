#ifdef WITH_REAL

#include <plinc/interp.h>
#include <plinc/fpmath.h>

#include <string.h> /*XXX*/
#include <stdio.h> /*XXX*/
#include <stdlib.h>
#include <math.h>



int
PlincFmtReal(float r, char *buf, int len)
{
    snprintf(buf, len, "%g", (double)r); /* XXX */
   
    return strlen(buf);
}



static void *
op_div(PlincInterp *i)
{
    PlincVal *v1, *v2, v;
    float a, b, c;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v1) == PLINC_TYPE_INT) {
            a = (float)(v1->Val.Int);
        } else if (PLINC_TYPE(*v1) == PLINC_TYPE_REAL) {
            a = v1->Val.Real;
        } else {
            return i->typecheck;
        }
            
        if (PLINC_TYPE(*v2) == PLINC_TYPE_INT) {
            b = (float)(v2->Val.Int);
        } else if (PLINC_TYPE(*v2) == PLINC_TYPE_REAL) {
            b = v2->Val.Real;
        } else {
            return i->typecheck;
        }
        
        PLINC_BEGINFP();
        
        c = a / b;
        
        if (PLINC_ENDFP(c)) {
            return i->undefinedresult;
        } else {
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_REAL;
            v.Val.Real = c;            

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);

            return NULL;
        }
    }
}



static void *
numf_op(PlincInterp *i, float (*func)(float f), void *errval)
{
    PlincVal *v;
    float f;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v) == PLINC_TYPE_INT) {
            f = (float)(v->Val.Int);
        } else if (PLINC_TYPE(*v) == PLINC_TYPE_REAL) {
            f = v->Val.Real;
        } else {
            return i->typecheck;
        }
            
        PLINC_BEGINFP();
        
        f = func(f);
        
        if (PLINC_ENDFP(f)) {
            return errval;
        } else {
            v->Flags = PLINC_ATTR_LIT | PLINC_TYPE_REAL;
            v->Val.Real = f;

            return NULL;
        }
    }
}



static void *
op_sqrt(PlincInterp *i)
{
    return numf_op(i, sqrtf, i->rangecheck);
}



static void *
op_ln(PlincInterp *i)
{
    return numf_op(i, logf, i->undefinedresult);
}



static void *
op_log(PlincInterp *i)
{
    return numf_op(i, log10f, i->undefinedresult);
}



static double
to_radians(float f)
{
    double d = f;
    
    return (d/360.0 - floor(d/360.0)) * 2.0 * M_PI;
}



static float
cos_deg(float f)
{
    return cos(to_radians(f));
}



static float
sin_deg(float f)
{
    return sin(to_radians(f));
}



static void *
op_cos(PlincInterp *i)
{
    return numf_op(i, cos_deg, i->undefinedresult);
}



static void *
op_sin(PlincInterp *i)
{
    return numf_op(i, sin_deg, i->undefinedresult);
}



static void *
numnum_op(PlincInterp *i, float (*func)(float f))
{
    PlincVal *v;
    float f;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v) == PLINC_TYPE_INT) {
            return NULL;
        } else if (PLINC_TYPE(*v) == PLINC_TYPE_REAL) {
            f = v->Val.Real;
        } else {
            return i->typecheck;
        }
            
        PLINC_BEGINFP();
        
        f = func(f);
        
        if (PLINC_ENDFP(f)) {
            return i->undefinedresult;
        } else {
            v->Flags = PLINC_ATTR_LIT | PLINC_TYPE_REAL;
            v->Val.Real = f;

            return NULL;
        }
    }
}



static void *
op_floor(PlincInterp *i)
{
    return numnum_op(i, floorf);
}



static void *
op_ceiling(PlincInterp *i)
{
    return numnum_op(i, ceilf);
}



static void *
op_round(PlincInterp *i)
{
    return numnum_op(i, rintf);
}



static float
realtrunc(float f)
{
    if (f < 0.0) {
        return ceilf(f);
    } else {
        return floorf(f);
    }
}



static void *
op_truncate(PlincInterp *i)
{
    return numnum_op(i, realtrunc);
}



static void *
op_atan(PlincInterp *i)
{
    PlincVal *v1, *v2, v;
    float a, b, c;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v1) == PLINC_TYPE_INT) {
            a = (float)(v1->Val.Int);
        } else if (PLINC_TYPE(*v1) == PLINC_TYPE_REAL) {
            a = v1->Val.Real;
        } else {
            return i->typecheck;
        }
            
        if (PLINC_TYPE(*v2) == PLINC_TYPE_INT) {
            b = (float)(v2->Val.Int);
        } else if (PLINC_TYPE(*v2) == PLINC_TYPE_REAL) {
            b = v2->Val.Real;
        } else {
            return i->typecheck;
        }
        
        PLINC_BEGINFP();
        
        c = atan2f(a, b);
        
        if (PLINC_ENDFP(c)) {
            return i->undefinedresult;
        } else {
            c = c * 180.0 / M_PI;
            if (c < 0) {
                c += 360.0;
            }
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_REAL;
            v.Val.Real = c;

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);

            return NULL;
        }
    }
}



static void *
op_exp(PlincInterp *i)
{
    PlincVal *v1, *v2, v;
    float a, b, c;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v1) == PLINC_TYPE_INT) {
            a = (float)(v1->Val.Int);
        } else if (PLINC_TYPE(*v1) == PLINC_TYPE_REAL) {
            a = v1->Val.Real;
        } else {
            return i->typecheck;
        }
            
        if (PLINC_TYPE(*v2) == PLINC_TYPE_INT) {
            b = (float)(v2->Val.Int);
        } else if (PLINC_TYPE(*v2) == PLINC_TYPE_REAL) {
            b = v2->Val.Real;
        } else {
            return i->typecheck;
        }
        
        PLINC_BEGINFP();
        
        c = powf(a, b);
        
        if (PLINC_ENDFP(c)) {
            return i->undefinedresult;
        } else {
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_REAL;
            v.Val.Real = c;

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);

            return NULL;
        }
    }
}



static const PlincOp ops[] = {
    {op_div,        "div"},
    {op_ceiling,    "ceiling"},
    {op_floor,      "floor"},
    {op_round,      "round"},
    {op_truncate,   "truncate"},
    {op_sqrt,       "sqrt"},
    {op_atan,       "atan"},
    {op_cos,        "cos"},
    {op_sin,        "sin"},
    {op_exp,        "exp"},
    {op_ln,         "ln"},
    {op_log,        "log"},

    {NULL,          NULL}
};



void
PlincInitRealOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

#endif /* WITH_REAL */

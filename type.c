#include <plinc/interp.h>

#include <stdlib.h>
#include <string.h>



static void *
op_type(PlincInterp *i)
{
    PlincVal *v;
     
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        PLINC_DECREF_VAL(*v);
        
        switch (PLINC_TYPE(*v)) {
        case PLINC_TYPE_INT:
            v->Val.Ptr = i->integertype;
            break;

        case PLINC_TYPE_REAL:
            v->Val.Ptr = i->realtype;
            break;

        case PLINC_TYPE_BOOL:
            v->Val.Ptr = i->booleantype;
            break;

        case PLINC_TYPE_ARRAY:
            v->Val.Ptr = i->arraytype;
            break;

        case PLINC_TYPE_STRING:
            v->Val.Ptr = i->stringtype;
            break;

        case PLINC_TYPE_NAME:
            v->Val.Ptr = i->nametype;
            break;

        case PLINC_TYPE_DICT:
            v->Val.Ptr = i->dicttype;
            break;

        case PLINC_TYPE_OP:
            v->Val.Ptr = i->operatortype;
            break;

        case PLINC_TYPE_FILE:
            v->Val.Ptr = i->filetype;
            break;

        case PLINC_TYPE_MARK:
            v->Val.Ptr = i->marktype;
            break;

        case PLINC_TYPE_NULL:
            v->Val.Ptr = i->nulltype;
            break;

        case PLINC_TYPE_SAVE:
            v->Val.Ptr = i->savetype;
            break;

        case PLINC_TYPE_FONTID:
            v->Val.Ptr = i->fonttype;
            break;
        }

        v->Flags = PLINC_TYPE_NAME;
        
        return NULL;
    }
}



static void *
setflags(PlincInterp *i, PlincUInt set, PlincUInt clear)
{
    PlincUInt32 *f;
    PlincVal *v;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_CAN_WRITE(*v)) {
            f = &(v->Flags);
            if (PLINC_TYPE(*v) == PLINC_TYPE_DICT) {
                f = &(((PlincDict *)(v->Val.Ptr))->Flags);
            }

            *f &= (~clear);
            *f |= set;

            return NULL;
        } else {
            return i->invalidaccess;
        }
    }
}



static void *
op_cvlit(PlincInterp *i)
{
    return setflags(i, PLINC_ATTR_LIT, 0);
}



static void *
op_cvx(PlincInterp *i)
{
    return setflags(i, 0, PLINC_ATTR_LIT);
}



static void *
op_dot_doexec(PlincInterp *i)
{
    return setflags(i, PLINC_ATTR_DOEXEC, PLINC_ATTR_LIT);
}



static void *
op_executeonly(PlincInterp *i)
{
    PlincUInt32 t;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        t = PLINC_TYPE(PLINC_OPTOPDOWN(i, 0));
        switch (t) {
        case PLINC_TYPE_ARRAY:
        case PLINC_TYPE_FILE:
        case PLINC_TYPE_STRING:
            return setflags(i, PLINC_ATTR_NOREAD | PLINC_ATTR_NOWRITE, 0);
        
        default:
            return i->typecheck;
        }
    }
}



static void *
only(PlincInterp *i, PlincUInt32 flags)
{
    PlincUInt32 t;

    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        t = PLINC_TYPE(PLINC_OPTOPDOWN(i, 0));
        switch (t) {
        case PLINC_TYPE_ARRAY:
        case PLINC_TYPE_DICT:
        case PLINC_TYPE_FILE:
        case PLINC_TYPE_STRING:
            return setflags(i, flags, 0);
        
        default:
            return i->typecheck;
        }
    }
}



static void *
op_noaccess(PlincInterp *i)
{
    return only(i, PLINC_ATTR_NOREAD | PLINC_ATTR_NOWRITE | PLINC_ATTR_NOEXEC);
}



static void *
op_readonly(PlincInterp *i)
{
    return only(i, PLINC_ATTR_NOWRITE);
}



static void *
check_not(PlincInterp *i, int flags)
{
    PlincUInt32 t;
    
    PlincVal v;
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        t = PLINC_TYPE(PLINC_OPTOPDOWN(i, 0));
        switch (t) {
        case PLINC_TYPE_ARRAY:
        case PLINC_TYPE_DICT:
        case PLINC_TYPE_FILE:
        case PLINC_TYPE_STRING:
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
            v.Val.Int = (PLINC_OPTOPDOWN(i, 0).Flags & flags) ? FALSE : TRUE;
        
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);

            return NULL;
        
        default:
            return i->typecheck;
        }
    }
}



static void *
op_xcheck(PlincInterp *i)
{
    return check_not(i, PLINC_ATTR_LIT);
}



static void *
op_rcheck(PlincInterp *i)
{
    return check_not(i, PLINC_ATTR_NOREAD);
}



static void *
op_wcheck(PlincInterp *i)
{
    return check_not(i, PLINC_ATTR_NOWRITE);
}



static const PlincOp ops[] = {
    {op_type,           "type"},
    {op_cvlit,          "cvlit"},
    {op_cvx,            "cvx"},
    {op_dot_doexec,     ".doexec"},
    {op_xcheck,         "xcheck"},
    {op_executeonly,    "executeonly"},
    {op_noaccess,       "noaccess"},
    {op_readonly,       "readonly"},
    {op_rcheck,         "rcheck"},
    {op_wcheck,         "wcheck"},

    {NULL,          NULL}
};



void
PlincInitTypeOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

/* $Endicor: exec.c,v 1.3 1999/01/17 05:14:05 tsarna Exp tsarna $ */

#include <plinc/interp.h>
#include <stdio.h> /*XXX*/


void *
PlincReprVal(PlincInterp *i, PlincVal *v)
{
    char *p;
    int j;
    
    switch (PLINC_TYPE(*v)) {
    case PLINC_TYPE_INT:
        fprintf(stderr, "%d", v->Val.Int);
        break;
        
    case PLINC_TYPE_REAL:
        fprintf(stderr, "%g", v->Val.Real);
        break;
        
    case PLINC_TYPE_ARRAY:
        /*XXX*/
        break;
        
    case PLINC_TYPE_BOOL:
        fprintf(stderr, "%s", v->Val.Int? "true" : "false");
        break;
        
    case PLINC_TYPE_STRING:
        j = PLINC_SIZE(*v);
        p = v->Val.Ptr;
        fputc('(', stderr);
        while (j) {
            switch (*p) {
            case '\n':
                fputs("\\n", stderr);
                break;

            case '(':
            case ')':
            case '\\':
                fputc('\\', stderr);
            
            default:
                fputc(*p, stderr);
            }
            p++, j--;
        }
        fputc(')', stderr);
        break;
        
    case PLINC_TYPE_NAME:
        if (PLINC_LIT(*v)) {
            fputc('/', stderr);
        }
        
        fwrite(((char*)(v->Val.Ptr))+1, *(unsigned char *)(v->Val.Ptr), 1, stderr);
        break;
        
    case PLINC_TYPE_DICT:
        /*XXX*/
        break;
        
    case PLINC_TYPE_OP:
        fprintf(stderr, "-%s-", v->Val.Op->Name);
        break;
        
    case PLINC_TYPE_FILE:
        fprintf(stderr, "-file-");
        break;
        
    case PLINC_TYPE_MARK:
        fprintf(stderr, "mark");
        break;

    case PLINC_TYPE_NULL:
        fprintf(stderr, "null");
        break;

    case PLINC_TYPE_SAVE:
        fprintf(stderr, "-save-");
        break;

    default:
        fprintf(stderr, "-value%02X-", PLINC_TYPE(*v) >> 20);
        break;
    }

    return NULL;
}



static void *
op_equalsequals(PlincInterp *i)
{
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        r = PlincReprVal(i, &PLINC_OPTOPDOWN(i, 0));
        fprintf(stderr, "\n");

        if (!r) {
            PLINC_OPPOP(i);
        }
        
        return r;
    }
}



static void *
op_pstack(PlincInterp *i)
{
    void *r = NULL;
    int j;
    
    for (j = 0; !r && (j < i->OpStack.Len); j++) {
        r = PlincReprVal(i, &PLINC_OPTOPDOWN(i, j));
        fprintf(stderr, "\n");
    }
    
    return r;
}



static PlincOp ops[] = {
    {"==",       op_equalsequals},
    {"pstack",   op_pstack},
    
    {NULL,       NULL}
};



void
PlincInitPrintOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

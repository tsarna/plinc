/* $Endicor: print.c,v 1.6 1999/01/24 03:47:42 tsarna Exp $ */

#include <plinc/interp.h>
#include <stdio.h> /*XXX*/


void *
PlincReprVal(PlincInterp *i, PlincVal *v)
{
    int j, k, first = TRUE;
    PlincDict *d;
    PlincVal *tv;
    char *p;
    
    switch (PLINC_TYPE(*v)) {
    case PLINC_TYPE_INT:
        fprintf(stderr, "%ld", v->Val.Int);
        break;
        
    case PLINC_TYPE_REAL:
        fprintf(stderr, "%g", v->Val.Real);
        break;
        
    case PLINC_TYPE_ARRAY:
        k = PLINC_SIZE(*v);
        if (PLINC_LIT(*v)) {
            fputc('[', stderr);
        } else {
            fputc('{', stderr);
        }
        for (j = 0; j < k; j++) {
            if (first) {
                first = FALSE;
            } else {
                fputc(' ', stderr);
            }
            
            tv = &(((PlincVal *)(v->Val.Ptr))[j]);
            PlincReprVal(i, tv);
        }
        if (PLINC_LIT(*v)) {
            fputc(']', stderr);
        } else {
            fputc('}', stderr);
        }
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
        fprintf(stderr, "<< ");
        d = v->Val.Ptr;
        for (j = 0; j < d->MaxLen; j++) {
            if (!PLINC_IS_NULL(d->Vals[j].Key)) {
                if (PLINC_TYPE(d->Vals[j].Key) == PLINC_TYPE_DICT) {
                    fputs("-dict-", stderr);
                } else {
                    PlincReprVal(i, &(d->Vals[j].Key));
                }
                fputc(' ', stderr);
                if (PLINC_TYPE(d->Vals[j].Val) == PLINC_TYPE_DICT) {
                    fputs("-dict-", stderr);
                } else {
                    PlincReprVal(i, &(d->Vals[j].Val));
                }
                fputc(' ', stderr);
            }
        }
        fprintf(stderr, ">>");
        break;
        
    case PLINC_TYPE_OP:
        fprintf(stderr, "--%s--", v->Val.Op->Name);
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



static const PlincOp ops[] = {
    {op_equalsequals,   "=="},
    {op_pstack,         "pstack"},
    
    {NULL,              NULL}
};



void
PlincInitPrintOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

#include <plinc/interp.h>
#include <plinc/file.h>

#include <string.h>
#include <stdio.h> /*XXX*/



static const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";



char *
PlincFmtRadix(PlincUInt i, PlincInt base, char *buf, int len)
{
    char *p;

    p = buf + len;
    *--p = '\0';

    if (i) {
        while (i) {
            *--p = digits[i % base];
            i /= base;
        }
    } else {
        *--p = '0';
    }
    
    return p;
}



char *
PlincFmtInt(PlincInt i, char *buf, int len)
{
    int neg = FALSE;
    PlincUInt v = i;
    char *p;

    if (i < 0) {
        neg = TRUE;
        i++; v = -i; v++;
    }
    
    p = buf + len;
    *--p = '\0';
    p[-1] = '0';
    
    if (v) {
        while (v) {
            *--p = digits[v % 10];
            v /= 10;
        }
    } else {
        *--p = '0';
    }

    if (neg) {
        *--p = '-';
    }
    
    return p;
}



void *
PlincFmtCVS(PlincInterp *i, PlincVal *v, char **buf, int *buflen)
{
    int len;
    char *p;
    
    if (PLINC_TYPE(*v) == PLINC_TYPE_OP) {
        p = v->Val.Op->Name;
        len = strlen(p);

        if ((len + 4) > *buflen) {
            return i->rangecheck;
        } else {
            memcpy(*buf, "--", 2);
            memcpy((*buf)+2, p, len);
            memcpy((*buf)+2+len, "--", 2);

            *buflen = len + 4;
        }
    } else if (PLINC_TYPE(*v) == PLINC_TYPE_BOOL) {
        *buf = (v->Val.Int) ? "true" : "false";
        *buflen = (v->Val.Int) ? 4 : 5;
    } else if (PLINC_TYPE(*v) == PLINC_TYPE_STRING) {
        *buf = (v->Val.Ptr);
        *buflen = PLINC_SIZE(*v);
    } else if (PLINC_TYPE(*v) == PLINC_TYPE_NAME) {
        *buf = (char *)(v->Val.Ptr) + 1;
        *buflen = *((unsigned char *)(v->Val.Ptr));
    } else if (PLINC_TYPE(*v) == PLINC_TYPE_INT) {
        p = PlincFmtInt(v->Val.Int, *buf, *buflen);
        *buflen = (*buflen - (p - *buf));
        *buf = p;
#ifdef WITH_REAL
    } else if (PLINC_TYPE(*v) == PLINC_TYPE_REAL) {
        *buflen = PlincFmtReal(v->Val.Real, *buf, *buflen);
#endif
    } else {
        *buf = "--nostringval--";
        *buflen = 15; /* strlen("--nostringval--") */
    }

    return NULL;
}



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
do_equals(PlincInterp *i, int newline)
{
    char buf[PLINC_FMT_BUFLEN], *p = buf;
    int len = PLINC_FMT_BUFLEN;
    PlincFile *f;
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        r = PlincFmtCVS(i, &PLINC_OPTOPDOWN(i, 0), &p, &len);
        if (!r) {
            f = (PlincFile *)(i->StdOut.Val.Ptr);
            len = PlincWriteString(f, p, len);
            if (len == PLINC_IOERR) {
                return i->ioerror;
            }

            if (newline) {
                len = PlincWrite(f, '\n');
                if (len == PLINC_IOERR) {
                    return i->ioerror;
                }
            }
                                                                                
            PLINC_OPPOP(i);
        }
        
        return r;
    }
}



static void *
op_equals(PlincInterp *i)
{
    return do_equals(i, TRUE);
}



static void *
op_equalsonly(PlincInterp *i)
{
    return do_equals(i, FALSE);
}



static void *
op_eqeqone(PlincInterp *i)
{
    union {
        /* we'll only use one or the other */
        char buf[PLINC_FMT_BUFLEN];
        PlincEncFile ef;
    } data;
    char *p = data.buf;
    int len = PLINC_FMT_BUFLEN;
    PlincFile *f;
    PlincVal *v;
    void *r = NULL;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        f = (PlincFile *)(i->StdOut.Val.Ptr);
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_IS_STRING(*v)) {
            PlincInitPStrEncode(&(data.ef), f, PLINC_ENCF_WITHBOD);
            len = PlincWriteString((PlincFile *)&(data.ef), v->Val.Ptr,
                PLINC_SIZE(*v));

            len = (len < PLINC_SIZE(*v)) ? PLINC_IOERR :
                PlincClose((PlincFile *)&(data.ef));
        } else {
            if (PLINC_IS_NAME(*v) && PLINC_LIT(*v)) {
                len = PlincWrite(f, '/');
                if (len) {
                    return i->ioerror;
                }
            }
            
            switch(PLINC_TYPE(*v)) {
            case PLINC_TYPE_ARRAY:
                p = "-array-"; len = 7;
                break;
        
            case PLINC_TYPE_DICT:
                p = "-dict-"; len = 6;
                break;
            
            case PLINC_TYPE_FILE:
                p = "-file-"; len = 6;
                break;
            
            case PLINC_TYPE_MARK:
                p = "-mark-"; len = 6;
                break;
            
            case PLINC_TYPE_NULL:
                p = "-null-"; len = 6;
                break;
            
            case PLINC_TYPE_SAVE:
                p = "-save-"; len = 6;
                break;

            default:            
                r = PlincFmtCVS(i, v, &p, &len);
            }
        
            if (!r) {
                len = PlincWriteString(f, p, len);
            }
        }
        
        if (!r) {
            if (len == PLINC_IOERR) {
                return i->ioerror;
            }   

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
    {op_equals,             "="},
    {op_equalsonly,         "=only"},
    {op_eqeqone,            "==one"},
    {op_pstack,             "pstack"},
    
    {NULL,                  NULL}
};



void
PlincInitPrintOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

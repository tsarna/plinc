#include <plinc/interp.h>
#include <plinc/file.h>

#include <plinc/token.h>

#include <stdlib.h>
#include <stdio.h>


#define WS  0x8000      /* Whitespace */
#define SP  0x4000      /* Special */
#define SN  0x2000      /* Start Number */
#define DG  0x1000      /* Digit */

#include "toktab.h"


#define isspace(c)      (_toktab[(unsigned char)(c)] & WS)
#define isspecial(c)    (_toktab[(unsigned char)(c)] & SP)
#define isendname(c)    (_toktab[(unsigned char)(c)] & (SP|WS))
#define isstartnum(c)   (_toktab[(unsigned char)(c)] & SN)
#define isdigit(c)      (_toktab[(unsigned char)(c)] & DG)
#define isoctal(c)      (isdigit(c) && ((c) < '8'))
#define val(c)          (_toktab[(unsigned char)(c)] & 0xFF)


static void    *NumToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v);
static int      IntToken(char **p, size_t *l, PlincInt *n, int base);
static void    *NameToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v);
static void    *StringToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v);


void *
PlincToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v)
{
    char *p = buf;
    size_t l = len;

    *eaten = 0;

    v->Flags = 0;

    while (l) {
        if (isspace(*p)) {
            do {
                p++; l--;
            } while (l && isspace(*p));
        } else if (*p == '%') {
            /* eat everything up until a newline */
            do {
                p++; l--;
            } while (l && (*p != '\n'));

            if (l) {
                /* eat the newline */
                p++; l--;
            }
        } else if (*p == '[') {
            v->Val.Ptr = i->LeftBracket;
            goto ok1;
        } else if (*p == ']') {
            v->Val.Ptr = i->RightBracket;
            goto ok1;
        } else if (*p == '{') {
            v->Val.Ptr = i->LeftBrace;
            goto ok2;
        } else if (*p == '}') {
            v->Val.Ptr = i->RightBrace;
            goto ok2;
        } else if (*p == '>') {
            p++; l--;
            if (l) {
                if  (*p == '>') {
                    v->Val.Ptr = i->RightAngleAngle;
                    goto ok1;
                } else {
                    return i->syntaxerror;
                }
            }
        } else if (*p == '<') {
            p++; l--;
            if (l) {
                if (*p == '<') {
                    v->Val.Ptr = i->LeftAngleAngle;
                    goto ok1;
                } else {
                    /*XXX base85 and hex strings */
                }
            }
        } else if (*p == '(') {
            p++; l--;
            *eaten = len - l;
                        
            return StringToken(i, p, l, eaten, v);
        } else if (isstartnum(*p)) {
            *eaten = len - l;
                        
            return NumToken(i, p, l, eaten, v);
        } else {
            *eaten = len - l;
                        
            return NameToken(i, p, l, eaten, v);
        }
    }

    return NULL;

ok2:
    v->Flags |= PLINC_ATTR_DOEXEC;

ok1:
    v->Flags |= PLINC_TYPE_NAME;
    l--;   
    
/*ok:*/
    *eaten = len - l;

    return i;        
}



static void *
NumToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v)
{
    int base, neg = FALSE;
    char *p = buf;
    size_t l = len;
    PlincUInt n;
    
    if (*p == '+') {
        p++; l--;
    } else if (*p == '-') {
        p++; l--;
        neg = TRUE;
    }
    
    if (IntToken(&p, &l, &n, 10)) {
        if (*p == '#') {
            base = n; n = 0;
            p++; l--;
            if (l) {
                if (IntToken(&p, &l, &n, base)) {
                    goto gotint;
                } else {
                    return i->syntaxerror;
                }
            }
        } else if ((*p == '.') || (*p == 'e') || (*p == 'E')) {
            /* XXX floats */
        } else {
            goto gotint;            
        }
    }

    return NULL;
    
gotint:
    v->Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
    
    if (neg) {
        v->Val.Int = -n;
    } else {
        v->Val.Int = n;
    }

    *eaten += (len - l);
            
    return i;
}



static int
IntToken(char **p, size_t *l, PlincInt *n, int base)
{
    *n = 0;
    
    while (l) {
        if (isdigit(**p) && (val(**p) < base)) {
            *n *= base;
            *n += val(**p);
            (*p)++; (*l)--;
        } else {
            return TRUE;
        }
    }
    
    return FALSE;
}


static void *
NameToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v)
{            
    char *ns, *p = buf;
    size_t nl, l = len;

    v->Flags = PLINC_TYPE_NAME;
            
    if (*p == '/') {
        v->Flags |= PLINC_ATTR_LIT;
        p++; l--;
    }

    ns = p; nl = 0;
    
    if (l) {
        while (l && !isendname(*p)) {
            l--; p++; nl++;
        }
        
        if (l) {
            *eaten += (len - l);
            
            v->Val.Ptr = PlincName(i->Heap, ns, nl);
            
            if (v->Val.Ptr) {
                return i;
            } else {
                return i->VMerror;
            }
        }
    }

    return NULL;
}


void *
PlincTokenVal(PlincInterp *i, PlincVal *vi, PlincVal *vo)
{
    size_t l, len;
    void *r;
    
    r = PlincToken(i, vi->Val.Ptr, PLINC_SIZE(*vi), &l, vo);
    if (r == i) {
        len = PLINC_SIZE(*vi);
        vi->Val.Ptr = (char *)(vi->Val.Ptr) + l;
        vi->Flags &= ~PLINC_SIZE_MASK;
        vi->Flags |= (len - l);
    }
       
    return r;
}


static void *
StringToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v)
{
    size_t sl = 0, l = len;
    char *p = buf, *s;
    int nest = 1;

    while (l && nest) {
        if (*p == '(') {
            nest++; sl++;
        } else if (*p == ')') {
            nest--; sl++;
        } else if (*p == '\\') {
            p++; l--;
            if (l) {
                if (*p == '\n') {
                    /* nothing */
                } else if (isoctal(*p)) {
                    if (l > 1) {
                        sl++;
                        if (isoctal(p[1])) {
                            p++; l--;
                            if (l > 1) {
                                if (isoctal(p[1])) {
                                    p++; l--;
                                }
                            } else {
                                return NULL;
                            }
                        }
                    } else {
                        return NULL;
                    }
                } else {
                    sl++;
                }
            } else {
                return NULL;
            }
        } else {
            sl++;
        }
        
        p++; l--;
    }
    
    if (nest) {
        return NULL;
    } else {
        sl--;
        
        if (sl > PLINC_MAXLEN) {
            return i->limitcheck;
        }
        
        s = PlincNewString(i->Heap, sl);
        
        v->Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | sl;
        v->Val.Ptr = s;
        
        p = buf;
        nest = 1;

        while (nest) {
            if (*p == '(') {
                nest++; *s++ = *p;
            } else if (*p == ')') {
                nest--;
                if (nest) {
                    *s++ = *p;
                }
            } else if (*p == '\\') {
                p++;
                if (*p == '\n') {
                    /* nothing */
                } else if (*p == 'n') {
                    *s++ = '\n';
                } else if (*p == 'r') {
                    *s++ = '\r';
                } else if (*p == 't') {
                    *s++ = '\t';
                } else if (*p == 'b') {
                    *s++ = '\x8';
                } else if (*p == 'f') {
                    *s++ = '\f';
                } else if (*p == '\\') {
                    *s++ = *p;
                } else if (isoctal(*p)) {
                    *s = val(*p);

                    if (isoctal(p[1])) {
                        p++;
                        *s <<= 3;
                        *s |= val(*p);
                        
                        if (isoctal(p[1])) {
                            p++;
                            *s <<= 3;
                            *s |= val(*p);
                        }
                    }

                    s++;
                } else {
                    *s++ = *p;
                }
            } else {
                *s++ = *p;
            }
        
            p++;
        }

        *eaten += (len - l);

        return i;
    }
}



/****************************************/



void *
PlincGetOther(PlincInterp *i, PlincFile *f, PlincVal *v, int c, int lit)
{
    return NULL;
}



void *
PlincGetString(PlincInterp *i, PlincFile *f, PlincVal *v,
void (*initfunc)(PlincDecodeFile *, PlincFile *, PlincUInt))
{
    PlincDecodeFile df;
    PlincUInt len, l;
    char *p;
    
    initfunc(&df, f, PLINC_DECF_WITHEOD);
    p = PlincBorrowMemory(i->Heap, &len);
    if (!p) {
        return i->VMerror;
    }
    
    l = PlincReadString((PlincFile *)&df, p, len);
    if (l == PLINC_IOERR) {
        return i->ioerror;
    } else if (l == PLINC_EOF) {
        return i->syntaxerror;
    } else if (l == len) {
        /* probably overfilled memory */
        PlincBorrowAbort(i->Heap, p, len);
        return i->VMerror;
    } else {
        p = PlincBorrowFinalize(i->Heap, p, len, l);
        if (!p) {
            return i->VMerror;
        }
        
        v->Flags = PLINC_TYPE_STRING | PLINC_ATTR_LIT | l;
        v->Val.Ptr = p;
        return NULL;
    }
}



void *
PlincGetToken(PlincInterp *i, PlincFile *f, PlincVal *val)
{
    PlincVal v;
    int c, depth = 0;
    void *r = NULL;
    
    do {
        c = PlincRead(f);
        switch (c) {
        case PLINC_EOF:
            r = i;
            goto done;

        case PLINC_IOERR:
            r = i->ioerror;
            goto done;
                        
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            continue;

        case '[':
            v.Flags = PLINC_TYPE_NAME;
            v.Val.Ptr = i->LeftBracket;
            goto value;

        case ']':
            v.Flags = PLINC_TYPE_NAME;
            v.Val.Ptr = i->RightBracket;
            goto value;
         
        case '<':
            c = PlincRead(f);
            if (c == '<') {
                v.Flags = PLINC_TYPE_NAME;
                v.Val.Ptr = i->LeftAngleAngle;
                goto value;
#if 0
            } else if (c == '~') {
                XXX
#endif
            } else {
                c = PlincUnRead(f, c);
                if (c) {
                    r = i->ioerror;
                    goto done;
                }
                r = PlincGetString(i, f, &v, PlincInitHexDecode);
                if (r) {
                    goto done;
                }
                goto value;
            }
            
        case '>':
            c = PlincRead(f);
            if (c == PLINC_IOERR) {
                r = i->ioerror;
                goto done;
            } else if (c == PLINC_EOF) {
                r = i->syntaxerror;
                goto done;
            } else if (c == '>') {
                v.Flags = PLINC_TYPE_NAME;
                v.Val.Ptr = i->RightAngleAngle;
                goto value;
            } else {
                c = PlincUnRead(f, c);
                if (c) {
                    r = i->ioerror;
                } else {
                    r = i->syntaxerror;
                }
                goto done;
            }
            
        case '(':
            r = PlincGetString(i, f, &v, PlincInitPStrDecode);
            if (r) {
                goto done;
            }
            goto value;
            
        case ')':
            r = i->syntaxerror;
            goto done;
            
        case '%':
            do {
                c = PlincRead(f);
                if (c == PLINC_EOF) {
                    r = i;
                    goto done;
                } else if (c == PLINC_IOERR) {
                    r = i->ioerror;
                    goto done;
                }
            } while ((c != '\r') && (c != '\n'));
	    continue;
	
        case '/':
            c = PlincRead(f);
            if (c == PLINC_IOERR) {
                r = i->ioerror;
                goto done;
            } else if (c == '/') {
                c = PlincRead(f);
                if (c == PLINC_IOERR) {
                    r = i->ioerror;
                    goto done;
                } else {
                    r = PlincGetOther(i, f, &v, c, 2);
                    if (r) {
                        goto done;
                    } else {
                        goto value;
                    }
                }
            } else {
                r = PlincGetOther(i, f, &v, c, 1);
                if (r) {
                    goto done;
                } else {
                    goto value;
                }
            }
            
        default:
            r = PlincGetOther(i, f, &v, c, 0);
            if (r) {
                goto done;
            } else {
                goto value;
            }
        }

        goto next;
        
    value:
        if (depth) {
            if (!PLINC_OPSTACKROOM(i, 1)) {
                return i->stackoverflow;
            } else {
                PLINC_OPPUSH(i, v);
            }
            continue;
        } else {
            *val = v;
            break;
        }
        
    done:
        /* unwind if depth */
        return r;
    
    next:
        ;
       
    } while (1);


    return NULL; /* XXX */
}



static void *
op_token(PlincInterp *i)
{
    PlincVal v, *vs;
    PlincStrFile sf;
    PlincFile *f;
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else if (!PLINC_OPSTACKROOM(i, 3)) {
        return i->stackoverflow;
    } else {
        vs = &PLINC_OPTOPDOWN(i, 0);
        if (PLINC_IS_FILE(*vs)) {
            f = (PlincFile *)(vs->Val.Ptr);
        } else if (PLINC_IS_STRING(*vs)) {
            PlincInitStrFile(&sf, vs);
            f = (PlincFile *)(void *)&sf;
        } else {
            return i->typecheck;
        }
            
        r = PlincGetToken(i, f, &v);
        if (r == i) {
            PLINC_OPPOP(i);
            v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
            v.Val.Int = FALSE;
            PLINC_OPPUSH(i, v);
            
            return NULL;
        } else if (r) {
            return r;
        } else if (PLINC_IS_FILE(*vs)) {
            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, v);
        } else {
            vs->Val.Ptr = (char *)(vs->Val.Ptr) + sf.Cur;
            PLINC_SET_SIZE(*vs, PLINC_SIZE(*vs) - sf.Cur);

            PLINC_OPPUSH(i, v);
        }

        v.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
        v.Val.Int = TRUE;
        PLINC_OPPUSH(i, v);
    }

    return NULL;
}



static const PlincOp ops[] = {
    {op_token,      "token"},

    {NULL,          NULL}
};



void
PlincInitTokenOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}


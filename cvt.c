#include <plinc/interp.h>

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <stdio.h> /* XXX */

static char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define BUFLEN      (sizeof(PlincInt) * CHAR_BIT + 2)


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



static int
radixdigit(char c, int base)
{
    int d;
    
    if ((c >= '0') && (c <= '9')) {
        d = c - '0';
    } else if ((c >= 'a') && (c <= 'z')) {
        d = c - 'a' + 10;
    } else if ((c >= 'A') && (c <= 'Z')) {
        d = c - 'A' + 10;
    } else {
        return -1;
    }
    
    if (d >= base) {
        return -1;
    }
    
    return d;
}



void *
PlincParseUInt(PlincInterp *i, PlincUInt *v, char *p, int len)
{
    PlincUInt n = 0;
    int base, d;
    
    if (!len) {
        /* number must not be empty or sign-only */
        return i->syntaxerror;
    }
    
    while (len && isdigit(*p)) {
        n *= 10;
        n += (*p - '0');
        p++; len--;
    }
    
    if (len) {
        if ((*p == '#') && (n >= 2) && (n <= 36)) {
            p++; len--;
            
            if (!len) {
                /* number must not be empty or sign-only */
                return i->syntaxerror;
            }
    
            base = n;
            n = d = 0;
            while (len && ((d = radixdigit(*p, base)) != -1)) {
                n *= base;
                n += d;
                p++; len--;
            }

            if (d < 0) {
                return i->syntaxerror;
            }
        } else {
            return i->syntaxerror;
        }
    }
    
    *v = n;

    return NULL;
}



void *
PlincParseInt(PlincInterp *i, PlincInt *v, char *p, int len)
{
    PlincUInt n = 0;
    int neg = FALSE;
    
    if (len) {
        if (*p == '-') {
            neg = TRUE;
            p++; len--;
        } else if (*p == '+') {
            p++; len--;
        }
    }
   
    if (!len) {
        /* number must not be empty or sign-only */
        return i->syntaxerror;
    }
    
    while (len && isdigit(*p)) {
        n *= 10;
        n += (*p - '0');
        p++; len--;
    }
    
    if (len) {
        return i->syntaxerror;
    } else {
        if (n > LONG_MAX) { /* XXX PLINCINT_MAX */
            return i->rangecheck;
        } else if (neg) {
            n--;
            *v = -((PlincInt)n);
            (*v)--;
        } else {
            *v = (PlincInt)n;
        }
    }
    
    return NULL;
}



void *
PlincParseNum(PlincInterp *i, PlincVal *v, char *p, int len)
{
    PlincInt t;
    void *r;
#ifdef WITH_REAL
    char buf[BUFLEN], *pe = NULL;
    double d;
#endif    

    r = PlincParseInt(i, &t, p, len);
    if (r == i->syntaxerror) {
        r = PlincParseUInt(i, (PlincInt *)(&t), p, len);
    }

    if (!r) {
        v->Val.Int = t;
        v->Flags = PLINC_TYPE_INT | PLINC_ATTR_LIT;
        
        return NULL;
#ifdef WITH_REAL
    } else if (r == i->syntaxerror) {
        len = min(len, BUFLEN-1);
        strncpy(buf, p, len);
        buf[len] = '\0';
        d = strtod(buf, &pe);
        if ((pe != buf) && (pe == &buf[len])) {
            v->Val.Real = d;
            v->Flags = PLINC_TYPE_REAL | PLINC_ATTR_LIT;
            
            return NULL;
        }
#endif
    }

    return r;
}



static void *
op_cvi(PlincInterp *i)
{
    PlincVal nv;
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        nv = PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(nv) == PLINC_TYPE_STRING) {
            if (PLINC_CAN_READ(nv)) {
                r = PlincParseNum(i, &nv, nv.Val.Ptr, PLINC_SIZE(nv));
                if (r) {
                    return r;
                }
            } else {
                return i->invalidaccess;
            }
        }
        
        if (PLINC_TYPE(nv) == PLINC_TYPE_INT) {
            /* done */
#ifdef WITH_REAL
        } else if (PLINC_TYPE(nv) == PLINC_TYPE_REAL) {
            nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
            nv.Val.Int = (PlincInt)(nv.Val.Real);
#endif
        } else {
            return i->typecheck;
        }

        PLINC_OPPOP(i);
        PLINC_OPPUSH(i, nv);
        
        return NULL;
    }
}



#ifdef WITH_REAL
static void *
op_cvr(PlincInterp *i)
{
    PlincVal nv;
    void *r;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        nv = PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(nv) == PLINC_TYPE_STRING) {
            if (PLINC_CAN_READ(nv)) {
                r = PlincParseNum(i, &nv, nv.Val.Ptr, PLINC_SIZE(nv));
                if (r) {
                    return r;
                }
            } else {
                return i->invalidaccess;
            }
        }
        
        if (PLINC_TYPE(nv) == PLINC_TYPE_REAL) {
            /* done */
        } else if (PLINC_TYPE(nv) == PLINC_TYPE_INT) {
            nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_REAL;
            nv.Val.Real = (float)(nv.Val.Int);
        } else {
            return i->typecheck;
        }

        PLINC_OPPOP(i);
        PLINC_OPPUSH(i, nv);
        
        return NULL;
    }
}
#endif



static void *
op_cvrs(PlincInterp *i)
{
    PlincVal *n, *rx, *s, nv;
    char buf[BUFLEN], *p;
    int base, len;
    PlincUInt u;
    
    if (!PLINC_OPSTACKHAS(i, 3)) {
        return i->stackunderflow;
    } else {
        n = &PLINC_OPTOPDOWN(i, 2);
        rx = &PLINC_OPTOPDOWN(i, 1);
        s = &PLINC_OPTOPDOWN(i, 0);
        
        if ((PLINC_TYPE(*s) != PLINC_TYPE_STRING)
        ||  (PLINC_TYPE(*rx) != PLINC_TYPE_INT)) {
            return i->typecheck;
        } else if (!PLINC_CAN_WRITE(*s)) {
            return i->invalidaccess;
        } else {
            base = rx->Val.Int;
            
            if ((base < 0) || (base > 36)) {
                return i->rangecheck;
            }
            
            if (base == 10) {
                if (PLINC_TYPE(*n) == PLINC_TYPE_INT) {
                    p = PlincFmtInt(n->Val.Int, buf, BUFLEN);
                    len = (BUFLEN - (p - buf));
#ifdef WITH_REAL
                } else if (PLINC_TYPE(*n) == PLINC_TYPE_REAL) {
                    len = PlincFmtReal(n->Val.Real, buf, BUFLEN);
                    p = buf;
#endif
                } else {
                    return i->typecheck;
                }
            } else {
                if (PLINC_TYPE(*n) == PLINC_TYPE_INT) {
                    u = (PlincUInt)(n->Val.Int);
#ifdef WITH_REAL
                } else if (PLINC_TYPE(*n) == PLINC_TYPE_REAL) {
                    u = (PlincUInt)((PlincInt)(n->Val.Real));
#endif
                } else {
                    return i->typecheck;
                }
                 
                p = PlincFmtRadix(u, base, buf, BUFLEN);
                len = (BUFLEN - (p - buf));
            }

            if (len > PLINC_SIZE(*s)) {
                return i->rangecheck;
            } else {
                memcpy(s->Val.Ptr, p, len);
                nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | len;
                nv.Val.Ptr = s->Val.Ptr;

                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, nv);
                
                return NULL;
            }
        }
    }
}



static void *
op_cvs(PlincInterp *i)
{
    PlincVal *v, *s, nv;
    char buf[BUFLEN], *p, *d;
    int len;

    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 1);
        s = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*s) != PLINC_TYPE_STRING) {
            return i->typecheck;
        } else if (!PLINC_CAN_WRITE(*s)) {
            return i->invalidaccess;
        } else if (PLINC_TYPE(*v) == PLINC_TYPE_OP) {
            p = v->Val.Op->Name;
            len = strlen(p);
            d = s->Val.Ptr;
            
            if ((len + 4) > PLINC_SIZE(*s)) {
                return i->rangecheck;
            } else {
                memcpy(d, "--", 2);
                memcpy(d+2, p, len);
                memcpy(d+2+len, "--", 2);

                nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | (len + 4);
                nv.Val.Ptr = d;
            }
        } else {
            if (PLINC_TYPE(*v) == PLINC_TYPE_BOOL) {
                p = (v->Val.Int) ? "true" : "false";
                len = (v->Val.Int) ? 4 : 5;
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_STRING) {
                p = (v->Val.Ptr);
                len = PLINC_SIZE(*v);
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_NAME) {
                p = (char *)(v->Val.Ptr) + 1;
                len = *((unsigned char *)(v->Val.Ptr));
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_INT) {
                p = PlincFmtInt(v->Val.Int, buf, BUFLEN);
                len = (BUFLEN - (p - buf));
#ifdef WITH_REAL
            } else if (PLINC_TYPE(*v) == PLINC_TYPE_REAL) {
                len = PlincFmtReal(v->Val.Real, buf, BUFLEN);
                p = buf;
#endif
            } else {
                p = "--nostringval--";
                len = 15; /* strlen("--nostringval--") */
            }

            if (len > PLINC_SIZE(*s)) {
                return i->rangecheck;
            } else {
                memcpy(s->Val.Ptr, p, len);
                nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | len;
                nv.Val.Ptr = s->Val.Ptr;
            }
        }

        PLINC_OPPOP(i);
        PLINC_OPPOP(i);
        PLINC_OPPUSH(i, nv);
                
        return NULL;
    }
}



static const PlincOp ops[] = {
    {op_cvi,            "cvi"},
    {op_cvr,            "cvr"},
    {op_cvrs,           "cvrs"},
    {op_cvs,            "cvs"},

    {NULL,              NULL}
};



void
PlincInitCvtOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}


/* $Endicor: token.c,v 1.1 1999/01/17 02:11:46 tsarna Exp tsarna $ */

#include <plinc/token.h>

#include <stdlib.h>
#include <stdio.h> /*XXX*/


#define WS  0x8000      /* Whitespace */
#define SP  0x4000      /* Special */
#define SN  0x2000      /* Start Number */
#define DG  0x1000      /* Digit */

#include "toktab.h"


#define isspace(c)      (_toktab[(unsigned char)(c)] & WS)
#define isspecial(c)    (_toktab[(unsigned char)(c)] & SP)
#define isendname(c)    (_toktab[(unsigned char)(c)] & SP|WS)
#define isstartnum(c)   (_toktab[(unsigned char)(c)] & SN)
#define isdigit(c)      (_toktab[(unsigned char)(c)] & DG)
#define val(c)          (_toktab[(unsigned char)(c)] & 0xFF)


static void    *NumToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v);
static int      IntToken(char **p, size_t *l, PlincInt *n, int base);
static void    *NameToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v);


void *
PlincToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v)
{
    char *p = buf;
    size_t l = len;

    *eaten = 0;

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
            goto ok1;
        } else if (*p == '}') {
            v->Val.Ptr = i->RightBrace;
            goto ok1;
        } else if (*p == '>') {
            return i->syntaxerror;
        } else if (*p == '<') {
            l--;
            if (l) {
                if (*p == '<') {
                    v->Val.Ptr = i->LeftAngleAngle;
                    goto ok1;
                } else {
                    /*XXX base85 and hex strings */
                }
            }
        } else if (*p == '(') {
            /*XXX*/
        } else if (isstartnum(*p)) {
            *eaten = len - l;
                        
            return NumToken(i, p, l, eaten, v);
        } else {
            *eaten = len - l;
                        
            return NameToken(i, p, l, eaten, v);
        }
    }

    return NULL;

ok1:
    v->Flags = PLINC_TYPE_NAME;
    l--;   
    
ok:
    *eaten = len - l;

    return i;        
}



static void *
NumToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v)
{
    int base, neg = FALSE;
    char *p = buf;
    size_t l = len;
    PlincInt n;
    
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
    v->Val.Int = n;

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

#include <plinc/interp.h>
#include <plinc/file.h>

#include <plinc/token.h>

#include <stdlib.h>
#include <stdio.h>


void *
PlincToken(PlincInterp *i, char *buf, size_t len, size_t *eaten, PlincVal *v)
{
    PlincStrFile sf;
    PlincFile *f;
    PlincVal vs;
    void *r;

    vs.Flags = PLINC_TYPE_STRING | (len);
    vs.Val.Ptr = buf;
    
    PlincInitStrFile(&sf, &vs);
    f = (PlincFile *)(void *)&sf;

    r = PlincGetToken(i, f, v);
    if (r == i) {
        return NULL;
    } else if (r) {
        return r;
    } else {
        *eaten = sf.Cur;
    }
    
    return i;
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




/****************************************/



void *
PlincGetOther(PlincInterp *i, PlincFile *f, PlincVal *v, int c, int lit)
{
    PlincUInt len, l = 0;
    void **link, *r;
    unsigned char *nlen;
    int go = TRUE;
    char *p;

    link = PlincBorrowMemory(i->Heap, &len);
    if (!link) {
        return i->VMerror;
    }
    
    nlen = (void *)(link + 1);
    p = (char *)(nlen + 1);

    do {
        switch (c) {
        case PLINC_IOERR:
            return i->ioerror;
            
        case '(':   case ')':   case '<':   case '>':   case '[':
        case ']':   case '{':   case '}':   case '/':   case '%':
            c = PlincUnRead(f, c);
            if (c) {
                return i->ioerror;
            }
            /* FALLTHROUGH */

        case PLINC_EOF:
        case '\0':  case '\t':  case '\n':
        case '\r':  case '\f':  case ' ':
            go = FALSE;
            break;

        default:
            if (l < len) {
                *p++ = c;
                l++;
            }
        }

        if (go) {
            c = PlincRead(f);
        }
    } while (go);
    
    if (!lit) {
        r = PlincParseNum(i, v, (nlen+1), l);

        if (!r || (r != i->syntaxerror)) {
            return r;
        }
    }
    
    *nlen = min(l, PLINC_MAXNAMELEN);
    v->Flags = PLINC_TYPE_NAME | l;
    r = PlincFindName(i->Heap->HeapHeader, (nlen+1), *nlen);
    if (r) {
        PlincBorrowAbort(i->Heap, link, len);
        v->Val.Ptr = r;
    } else {
        link = PlincBorrowFinalize(i->Heap, link, len, l + sizeof(void *) + 1);
        if (link) {
            v->Val.Ptr = (link + 1);
            PLINC_LINK(v->Val.Ptr) = i->Heap->HeapHeader->Names;
            i->Heap->HeapHeader->Names = v->Val.Ptr;
        } else {
            return i->VMerror;
        }
    }
    
    if (lit == 1) {
        v->Flags |= PLINC_ATTR_LIT;
    } else if (lit == 2) {
        r = PlincLoadDict(i, v, v);
        if (r) {
            return r;
        }
    }
    
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
    
    l = PlincReadString((PlincFile *)(void *)(&df), p, len);
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
    int c, depth = 0, len = 0;
    void *r = NULL;
    
    do {
        c = PlincRead(f);
        switch (c) {
        case PLINC_EOF:
            r = i;
            break;

        case PLINC_IOERR:
            r = i->ioerror;
            break;
                        
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            continue;

        case '[':
            v.Flags = PLINC_TYPE_NAME;
            v.Val.Ptr = i->LeftBracket;
            break;

        case ']':
            v.Flags = PLINC_TYPE_NAME;
            v.Val.Ptr = i->RightBracket;
            break;
         
        case '{':
            if (depth) {
                if (!PLINC_STACKROOM(i->ExecStack, 1)) {
                    r = i->stackoverflow;
                    break;
                }
                i->ExecStack.Len++;
                PLINC_TOPDOWN(i->ExecStack, 0).Val.Int = len;
                len = 0;
            }
            depth++;
            continue;

        case '}':
            if (depth) {
                depth--;
                v.Val.Ptr = PlincNewArray(i->Heap, len);
                if (v.Val.Ptr) {
                    v.Flags = PLINC_TYPE_ARRAY | len;
                    while (len) {
                        len--;
                        PlincPutArray(i, v.Val.Ptr, len, &PLINC_OPTOPDOWN(i, 0));
                        PLINC_OPPOP(i);
                    }
                    if (depth) {
                        PLINC_OPPUSH(i, v);
                    }
                } else {
                    r = i->VMerror;
                    break;
                }

                if (depth) {
                    len = PLINC_TOPDOWN(i->ExecStack, 0).Val.Int + 1;
                    i->ExecStack.Len--;
                    continue;
                } else {
                    break;
                }
            } else {
                r = i->syntaxerror;
                break;
            }
         
        case '<':
            c = PlincRead(f);
            if (c == '<') {
                v.Flags = PLINC_TYPE_NAME;
                v.Val.Ptr = i->LeftAngleAngle;
                break;
#if 0
            } else if (c == '~') {
                XXX
#endif
            } else {
                c = PlincUnRead(f, c);
                if (c) {
                    r = i->ioerror;
                    break;
                }
                r = PlincGetString(i, f, &v, PlincInitHexDecode);
                break;
            }
            
        case '>':
            c = PlincRead(f);
            if (c == PLINC_IOERR) {
                r = i->ioerror;
            } else if (c == PLINC_EOF) {
                r = i->syntaxerror;
            } else if (c == '>') {
                v.Flags = PLINC_TYPE_NAME;
                v.Val.Ptr = i->RightAngleAngle;
            } else {
                c = PlincUnRead(f, c);
                if (c) {
                    r = i->ioerror;
                } else {
                    r = i->syntaxerror;
                }
            }
            break;
            
        case '(':
            r = PlincGetString(i, f, &v, PlincInitPStrDecode);
            break;
            
        case ')':
            r = i->syntaxerror;
            break;
            
        case '%':
            do {
                c = PlincRead(f);
                if (c == PLINC_EOF) {
                    r = i;
                } else if (c == PLINC_IOERR) {
                    r = i->ioerror;
                }
            } while (!r && (c != '\r') && (c != '\n'));
            if (r) {
                break;
            }
	    continue;
	
        case '/':
            c = PlincRead(f);
            if (c == PLINC_IOERR) {
                r = i->ioerror;
            } else if (c == '/') {
                c = PlincRead(f);
                r = PlincGetOther(i, f, &v, c, 2);
            } else {
                r = PlincGetOther(i, f, &v, c, 1);
            }
            break;
            
        default:
            r = PlincGetOther(i, f, &v, c, 0);
            break;
        }

        /* ok, now do something with the value we got */

        if (!r) {
            if (depth) {
                if (!PLINC_OPSTACKROOM(i, 1)) {
                    r = i->stackoverflow;
                    break;
                } else {
                    PLINC_OPPUSH(i, v);
                    len++;
                }
                continue;
            } else {
                *val = v;
                return NULL;
            }
        }
        
        if (r) {
            while (depth) {
                while (len) {
                    PLINC_OPPOP(i);
                    len--;
                }
                depth--;
                if (depth) {
                    len = PLINC_TOPDOWN(i->ExecStack, 0).Val.Int;
                    i->ExecStack.Len--;
                }
            }
        }
    } while (!r);

    return r;
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


#include <plinc/interp.h>
#include <plinc/file.h>

#include <stdlib.h>
#include <stdio.h>


PlincInt
PlincEditLine(PlincInterp *i, char *buf, PlincUInt len)
{
    PlincFile *fi, *fo;
    char *p = buf;
    int c, l = 0;
    PlincInt r;

    fi = i->StdIn.Val.Ptr;
    fo = i->StdOut.Val.Ptr;
    
    while ((p-buf) < len) {
        c = fi->Ops->read(fi);
        switch (c) {
        case '\b':      /* BS */
        case '\x7f':    /* DEL */
            if (p > buf) {
                if (i->Flags & PLINC_FLAG_ECHO) {
                    r = fo->Ops->writestring(fo, "\b \b", 3);
                } else {
                    r = fo->Ops->writestring(fo, " \b", 2);
                }
                if (r < 0) {
                    return r;
                }
                p--;
            }
            break;

        case '\x12':    /* ^R */
            l = (p - buf);
            while (l) {
                r = fo->Ops->writestring(fo, "\b \b", 3);
                if (r < 0) {
                    return r;
                }
                l--;
            }
            r = fo->Ops->writestring(fo, buf, p-buf);
            if (r < 0) {
                return r;
            }
            break;
            
        case '\x15':    /* ^U */
            while (p > buf) {
                r = fo->Ops->writestring(fo, "\b \b", 3);
                if (r < 0) {
                    return r;
                }
                p--;
            }
            break;

        case '\r':
        case '\n':
            if (i->Flags & PLINC_FLAG_ECHO) {
                r = fo->Ops->write(fo, '\n');
                if (r < 0) {
                    return r;
                }
            }
            return (p - buf);            
            
        case '\x3':     /* ^C */
        case '\x4':     /* ^D XXX temporary */ 
        case PLINC_EOF:
            return PLINC_EOF;
            
        case PLINC_IOERR:
            return PLINC_IOERR;
            
        default:
            if (i->Flags & PLINC_FLAG_ECHO) {
                r = fo->Ops->write(fo, c);
                if (r < 0) {
                    return r;
                }
            }
            *p++ = c;
            break;
        }
    }

    return (p - buf);
}



PlincInt
PlincEditStatement(PlincInterp *i, char *buf, PlincUInt len)
{
    char *p = buf;
    PlincInt r, l = len;
    
    do {
        if (!l) {
            /* no space left? return incomplete statement */
            return (p - buf);
        }
        
        r = PlincEditLine(i, p, l);
        if (r < 0) {
            return r;
        }
        p += r; l -= r;
        if (l) {
            *p++ = '\n';
            l--;
        }
    } while (!PlincStatementComplete(buf, p - buf));
    
    return (p - buf);  
}



static char *
PlincHEXComplete(char *p, char *end)
{
    while (p < end) {
        if (*p++ == '>') {
            return p;
        }
    }
    
    return NULL;
}



static char *
PlincB85Complete(char *p, char *end)
{
    while (p < end) {
        if (*p++ == '~') {
            if (p >= end) {
                return NULL;
            } else if (*p++ == '>') {
                return p;
            }
        }
    }
    
    return NULL;
}



static char *
PlincStringComplete(char *p, char *end)
{
    int nest = 1; /* already seen when we get here */
    
    while (p < end) {
        switch (*p++) {
        case '(':
            nest++;
            break;
            
        case ')':
            nest--;
            if (!nest) {
                return p;
            }
            break;

        case '\\':
            if (p >= end) {
                return NULL;
            } else {
                p++;
            }
            break;
        }
    }
    
    return NULL;
}



int
PlincStatementComplete(char *buf, PlincUInt len)
{
    char *p = buf, *end;
    int nest = 0;
    
    end = p + len;
    while (p < end) {
        switch (*p) {
        case '{':
            nest++;
            break;
        
        case '}':
            nest--;
            break;

        case '(':
            p++;
            if (p >= end) {
                return FALSE;
            }
            p = PlincStringComplete(p, end);
            if (!p) {
                return FALSE;
            }
            continue;
            
        case '<':
            p++;
            if (p >= end) {
                return FALSE;
            }
            if (*p == '~') {
                p = PlincB85Complete(p, end);
            } else {
                p = PlincHEXComplete(p, end);
            }
            if (!p) {
                return FALSE;
            }
            continue;
        }
        
        p++;
    }
    
    return (nest <= 0) ? TRUE : FALSE;
}

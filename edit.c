#include <plinc/interp.h>
#include <plinc/file.h>


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

#include <plinc/types.h>
#include <plinc/file.h>


/*
 *  default readline implementation for a file that only provides read
 *
 *  l is a pointer to the size of the buffer on input, and
 *  is changed to the ammount actually read on return.
 *
 *  returns: TRUE if read a line, PLINC_EOF if hit EOF before EOL,
 *  PLINC_IOERR on error, FALSE if filled buffer before hitting EOL.
 */
PlincInt
plinc_io_readline(PlincFile *f, char *buf, PlincInt *l)
{
    int c, len;
    
    len = *l;
    *l = 0;
    
    while (len) {
        c = f->Ops->read(f);
        if (c < 0) { /* IOERR or EOF */
            return c;
        } else {
            if (c == '\n') {
                return TRUE;
            } else if (c == '\r') {
                c = f->Ops->read(f);
                if (c == PLINC_EOF) {
                    return TRUE; /* CR was EOL */
                } else if (c == PLINC_IOERR) {
                    return c;
                } else if (c == '\n') {
                    return TRUE;
                } else {
                    /* CR was EOL, whatever we got starts the next line */
                    c = f->Ops->unread(f, c);
                    if (c) {
                        return c;
                    }
                    return TRUE;
                }
            } else {
                *buf++ = (char)c;
                (*l)++; len--;
            }
        }
    }
    
    return FALSE;   /* filled before EOL */
}

#include <plinc/types.h>
#include <plinc/file.h>


/*
 *  default readstring implementation for a file that only provides read
 */
PlincInt
plinc_io_readstring(PlincFile *f, char *buf, PlincInt l)
{
    PlincInt r = 0;
    int c;
    
    while (l) {
        c = f->Ops->read(f);
        if (c == PLINC_IOERR) {
            return PLINC_IOERR;
        } else if (c == PLINC_EOF) {
            return r;
        } else {
            *buf++ = (char)c;
            l--; r++;
        }
    }
    
    return r;
}

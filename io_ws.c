#include <plinc/types.h>
#include <plinc/file.h>


/*
 *  default writestring implementation for a file that only provides read
 */
PlincInt
plinc_io_writestring(PlincFile *f, char *buf, PlincInt l)
{
    PlincInt c, r = 0;
    
    while (l) {
        c = f->Ops->write(f, *buf++);
        if (c) {
            return c;
        } else {
            l--; r++;
        }
    }
    
    return r;
}

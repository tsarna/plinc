#include <plinc/file.h>
#include <stdio.h>


static int
stdio_read(PlincFile *f)
{
    int c;
    
    c = getc((FILE *)(f->Ptr));
    if (c == EOF) {
        if (feof((FILE *)(f->Ptr))) {
            return PLINC_EOF;
        } else {
            return PLINC_IOERR;
        }
    } else {
        return c;
    }
}



static int
stdio_write(PlincFile *f, int c)
{
    return putc(c, ((FILE *)(f->Ptr))) ? PLINC_IOERR : 0;
}



PlincFileOps stdio_ops = {
    stdio_read,
    stdio_write
};

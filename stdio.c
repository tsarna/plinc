#include <plinc/types.h>
#include <plinc/file.h>
#include <stdio.h>


static int
stdio_close(PlincFile *f)
{
    int c = 0;
    
    c = fclose((FILE *)(f->Ptr));
    if (c == EOF) {
        c = PLINC_IOERR;
    }

    f->Ptr = NULL;
    f->Ops = &plinc_closed_ops;
    
    return c;
}



static int
stdio_flush(PlincFile *f)
{
    int c = 0;
    
    c = fflush((FILE *)(f->Ptr));
    if (c == EOF) {
        return PLINC_IOERR;
    } else {
        return 0;
    }
}



static int
stdio_reset(PlincFile *f)
{
#if defined(__NetBSD__)
    int c = 0;
    
    c = fpurge((FILE *)(f->Ptr));
    if (c == EOF) {
        return PLINC_IOERR;
    } else
#endif
    {
            return 0;
    }
}



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
    return putc(c, ((FILE *)(f->Ptr))) == EOF ? PLINC_IOERR : 0;
}



static PlincInt
stdio_readstring(PlincFile *f, char *buf, PlincInt l)
{
    size_t r;
    
    clearerr((FILE *)(f->Ptr));
    r = fread(buf, 1, l, (FILE *)(f->Ptr));
    if (ferror((FILE *)(f->Ptr))) {
        return PLINC_IOERR;
    }
    
    return r;
}



static int
stdio_unread(PlincFile *f, int c)
{
    if (ungetc(c, (FILE *)(f->Ptr)) == EOF) {
        return PLINC_IOERR;
    } else {
        return 0;
    }
}



static PlincInt
stdio_writestring(PlincFile *f, char *buf, PlincInt l)
{
    size_t r;
    
    clearerr((FILE *)(f->Ptr));
    r = fwrite(buf, 1, l, (FILE *)(f->Ptr));
    if (ferror((FILE *)(f->Ptr))) {
        return PLINC_IOERR;
    }
    
    return r;
}



PlincFileOps stdio_ops = {
    stdio_close,
    stdio_flush,
    stdio_reset,
    plinc_io_bytesavailable,
    stdio_read,
    stdio_readstring,
    plinc_io_readline,
    stdio_unread,
    stdio_write,
    stdio_writestring,
};

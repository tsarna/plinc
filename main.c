#include <plinc/interp.h>
#include <plinc/file.h>

#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#define HEAPSIZE 65536

extern const PlincFileOps stdio_ops;

int
main(int argc, char *argv[])
{
    struct termios orig, new;
    PlincFile si, so;
    PlincInterp *i;
    char buf[1024];
    void *r;
    int l;

    si.Ops = &stdio_ops;
    si.Ptr = stdin;
    so.Ops = &stdio_ops;
    so.Ptr = stdout;
    
    i = PlincNewInterp(HEAPSIZE);
    if (i) {
        i->StdIn.Flags = PLINC_TYPE_FILE | PLINC_ATTR_NOWRITE
            | PLINC_ATTR_LIT;
        i->StdIn.Val.Ptr = &si;
        
        i->StdOut.Flags = PLINC_TYPE_FILE | PLINC_ATTR_NOREAD
            | PLINC_ATTR_NOEXEC | PLINC_ATTR_LIT;
        i->StdOut.Val.Ptr = &so;
        
        tcgetattr(STDIN_FILENO, &orig);
        new = orig;
        cfmakeraw(&new);
#if 0
        new.c_iflag |= ICRNL;
#endif
        new.c_oflag = OPOST|ONLCR;
        tcsetattr(STDIN_FILENO, TCSANOW, &new);
    
        r = PlincExecStr(i, "prompt\n");
        while ((l = PlincEditStatement(i, buf, sizeof(buf) - 1)) >= 0) {
            buf[l++] = '\0';
            r = PlincExecStr(i, buf);
            if (r) {
                fprintf(stderr, "ERROR: %p ", r);
                fwrite(((char*)(r))+1, *(unsigned char *)(r), 1, stderr);
                fprintf(stderr, "\n");
            }
            r = PlincExecStr(i, "prompt\n");
        }
        
        tcsetattr(STDIN_FILENO, TCSANOW, &orig);

        PlincFreeInterp(i);
    } else {
        return 1;
    }
    
    return 0;
}

#include <plinc/interp.h>
#include <plinc/file.h>
#include <stdio.h>

#define HEAPSIZE 65536

extern const PlincFileOps stdio_ops;

int
main(int argc, char *argv[])
{
    PlincInterp *i;
    char buf[256];
    void *r;

    PlincFile si, so;

    si.Ops = &stdio_ops;
    si.Ptr = stdin;
    so.Ops = &stdio_ops;
    so.Ptr = stdout;
    
    i = PlincNewInterp(HEAPSIZE);
    if (i) {
/*        fwrite(i->Heap->HeapHeader, HEAPSIZE, 1, stdout);*/

        i->StdIn.Flags = PLINC_TYPE_FILE | PLINC_ATTR_NOWRITE
            | PLINC_ATTR_LIT;
        i->StdIn.Val.Ptr = &si;
        
        i->StdOut.Flags = PLINC_TYPE_FILE | PLINC_ATTR_NOREAD
            | PLINC_ATTR_NOEXEC | PLINC_ATTR_LIT;
        i->StdOut.Val.Ptr = &so;
        
        r = PlincExecStr(i, "prompt\n");
        while (fgets(buf, sizeof(buf), stdin)) {
            r = PlincExecStr(i, buf);
            if (r) {
                fprintf(stderr, "ERROR: %p ", r);
                fwrite(((char*)(r))+1, *(unsigned char *)(r), 1, stderr);
                fprintf(stderr, "\n");
            }
        r = PlincExecStr(i, "prompt\n");
/*            fprintf(stderr, "> ");*/
        }
        
        PlincFreeInterp(i);
    } else {
        return 1;
    }
    
    return 0;
}

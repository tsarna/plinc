/* $Endicor: main.c,v 1.3 1999/01/17 04:58:16 tsarna Exp $ */

#include <plinc/interp.h>
#include <stdio.h>

#define HEAPSIZE 65536

int
main(int argc, char *argv[])
{
    PlincInterp *i;
    char buf[256];
    void *r;
    
    i = PlincNewInterp(HEAPSIZE);
    if (i) {
        fwrite(i->Heap->HeapHeader, HEAPSIZE, 1, stdout);

        PlincPrintDict(i, i->systemdict);

        while (fgets(buf, sizeof(buf), stdin)) {
            r = PlincExecStr(i, buf);
            if (r) {
                fprintf(stderr, "ERROR: %p ", r);
                fwrite(((char*)(r))+1, *(unsigned char *)(r), 1, stderr);
                fprintf(stderr, "\n");
            }
        }
        
        PlincFreeInterp(i);
    } else {
        return 1;
    }
    
    return 0;
}

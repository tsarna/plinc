/* $Endicor: main.c,v 1.1 1999/01/14 01:26:00 tsarna Exp $ */

#include <plinc/interp.h>
#include <stdio.h>

#define HEAPSIZE 65536

int
main(int argc, char *argv[])
{
    PlincInterp *i;
    
    i = PlincNewInterp(HEAPSIZE);
    if (i) {
        fwrite(i->Heap->HeapHeader, HEAPSIZE, 1, stdout);

        PlincPrintDict(i, i->systemdict);
        
        PlincFreeInterp(i);
    } else {
        return 1;
    }
    
    return 0;
}

/* $Endicor$ */

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
        
        PlincFreeInterp(i);
    } else {
        return 1;
    }
    
    return 0;
}

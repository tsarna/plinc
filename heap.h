/* $Endicor: heap.h,v 1.2 1999/01/12 23:13:28 tsarna Exp tsarna $ */

#ifndef PLINC_HEAP_H
#define PLINC_HEAP_H

#include <plinc/types.h>


#define PLINC_NAME_MAX  127


typedef struct _PlincHeapHeader PlincHeapHeader;
struct _PlincHeapHeader {
    size_t      Len;
    size_t      Left;
    void       *Top;
    void       *Names;
    void       *Objects;
};


typedef struct _PlincHeap PlincHeap;
struct _PlincHeap {
    PlincHeapHeader    *HeapHeader;
};


/****************************************/

void        PlincInitHeapHeader(PlincHeapHeader *mem, size_t size);
PlincHeap  *PlincNewHeap(size_t size);
void        PlincFreeHeap(PlincHeap *h);
void       *PlincAllocHeap(PlincHeap *h, size_t len);
void       *PlincName(PlincHeap *h, char *name, size_t len);

#endif /* PLINC_HEAP_H */

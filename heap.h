/* $Endicor: heap.h,v 1.1 1999/01/12 22:27:34 tsarna Exp tsarna $ */


#include <plinc/types.h>


#define PLINC_NAME_MAX  127


typedef struct _PlincHeapHeader PlincHeapHeader;
struct _PlincHeapHeader {
    size_t      Len;
    size_t      Left;
    void       *Top;
    char       *Names;
    char       *Objects;
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

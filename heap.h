/* $Endicor$ */


#include <plinc/types.h>


#define PLINC_NAME_MAX  127


typedef struct _PlincHeapHeader PlincHeapHeader;
struct _PlincHeapHeader {
    PlincUInt   Magic1;
    PlincUInt   Magic2;
#define         PLINC_MAGIC1    0x4E444352
#define         PLINC_MAGIC2    0x504C4E43

    PlincUInt   Version;
    PlincUInt   Len;
    PlincUInt   Top;
    PlincPtr    Names;
    PlincPtr    Objects;
};


typedef struct _PlincHeap PlincHeap;
struct _PlincHeap {
    PlincHeapHeader    *HeapHeader;
};


/****************************************/

void        PlincInitHeapHeader(PlincHeapHeader *mem, size_t size);
PlincHeap  *PlincNewHeap(size_t size);
void        PlincFreeHeap(PlincHeap *h);
PlincPtr    PlincAllocHeap(PlincHeap *h, size_t len);
PlincPtr    PlincName(PlincHeap *h, char *name, size_t len);

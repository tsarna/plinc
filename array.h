/* $Endicor: array.h,v 1.2 1999/01/12 23:13:28 tsarna Exp $ */

#include <plinc/types.h>
#include <plinc/heap.h>


typedef struct _PlincArray PlincArray;
struct _PlincArray {
    PlincUInt   Flags;
};

/****************************************/

void   *PlincNewArray(PlincHeap *h, PlincUInt size);

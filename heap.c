/* $Endicor$ */


#include <stdlib.h>
#include <stdio.h> /*XXX*/

#include <plinc/heap.h>


void
PlincInitHeapHeader(PlincHeapHeader *hh, size_t size)
{
    hh->Magic1 = PLINC_MAGIC1;
    hh->Magic2 = PLINC_MAGIC2;
    hh->Version = 0x00010000;
    hh->Len = size;
    hh->Top = PLINC_ROUND(sizeof(PlincHeapHeader));
    hh->Objects = PLINC_NULL;
    hh->Names = PLINC_NULL;
}



PlincHeap *
PlincNewHeap(size_t size)
{
    PlincHeapHeader *hh;
    PlincHeap *h;
    
    h = malloc(sizeof(PlincHeap));
    hh = malloc(size);
    
    if (hh && h) {
        h->HeapHeader = hh;
        PlincInitHeapHeader(hh, size);
    } else {
        if (h) free(h);
        if (hh) free(hh);
    }
    
    return h;
}



PlincPtr
PlincAllocHeap(PlincHeap *h, size_t len)
{
    PlincPtr r = PLINC_NULL;
    PlincHeapHeader *hh = h->HeapHeader;

    len = PLINC_ROUND(len + sizeof(PlincPtr));
   
    if (len <= (hh->Len - hh->Top)) {
        r = hh->Top + sizeof(PlincPtr);;
        hh->Top += len;
    }
  
    return r;
}



void
PlincFreeHeap(PlincHeap *h)
{
    if (h) {
        if (h->HeapHeader) {
            free(h->HeapHeader);
        }
        
        free(h);
    }
}


PlincPtr
PlincName(PlincHeap *h, char *name, size_t len)
{
    PlincPtr n, r = PLINC_NULL;
    PlincHeapHeader *hh = h->HeapHeader;
    unsigned char *s;
    size_t nlen;
    
    if (len <= PLINC_NAME_MAX) {
        n = hh->Names;
        while (n) {
            s = (unsigned char *)PLINC_CPTR(hh, n);
            nlen = *s++;
            if (nlen == len) {
                if (!memcmp(s, name, len)) {
                    r = n;
                    break;
                }
            }
       
            n = PLINC_LINK(hh, n);
        }
    
        if (!r) {
            r = PlincAllocHeap(h, len + 1);
            if (r) {
                unsigned char *cp;
                PlincPtr *p;
               
                PLINC_LINK(hh, r) = hh->Names;
                hh->Names = r;
                cp = (unsigned char *)PLINC_CPTR(hh, r);
                *cp++ = len;
                memcpy(cp, name, len);
            }
        }
    }

    return r;
}

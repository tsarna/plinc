/* $Endicor: heap.c,v 1.1 1999/01/12 22:27:34 tsarna Exp tsarna $ */


#include <stdlib.h>
#include <stdio.h> /*XXX*/

#include <plinc/heap.h>


void
PlincInitHeapHeader(PlincHeapHeader *hh, size_t size)
{
    hh->Len = size;
    hh->Left = size - PLINC_ROUND(sizeof(PlincHeapHeader));
    hh->Top = ((char *)hh) + PLINC_ROUND(sizeof(PlincHeapHeader));
    hh->Objects = NULL;
    hh->Names = NULL;
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



void *
PlincAllocHeap(PlincHeap *h, size_t len)
{
    PlincHeapHeader *hh = h->HeapHeader;
    char **r = NULL;

    len = PLINC_ROUND(len + sizeof(char *));
   
    if (len <= hh->Left) {
        r = ((char **)hh->Top) + 1;
        hh->Top = ((char *)hh->Top) + len;
    }

    return (void *)r;
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


void *
PlincName(PlincHeap *h, char *name, size_t len)
{
    PlincHeapHeader *hh = h->HeapHeader;
    unsigned char *n, *r = NULL;
    size_t nlen;
    
    if (len <= PLINC_NAME_MAX) {
        n = hh->Names;
        while (n) {
            nlen = *n;
            if (nlen == len) {
                if (!memcmp(n+1, name, len)) {
                    r = n;
                    break;
                }
            }
       
            n = PLINC_LINK(n);
        }
    
        if (!r) {
            r = PlincAllocHeap(h, len + 1);
            if (r) {
                unsigned char *cp;
               
                PLINC_LINK(r) = hh->Names;
                hh->Names = r;
                cp = r;
                *cp++ = len;
                memcpy(cp, name, len);
            }
        }
    }

    return r;
}

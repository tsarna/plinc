#include <stdlib.h>
#include <string.h>

#include <plinc/interp.h>


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
PlincAllocHeapLinked(PlincHeap *h, size_t len)
{
    PlincHeapHeader *hh = h->HeapHeader;
    char **r = NULL;

    len = PLINC_ROUND(len + sizeof(char *));
   
    if (len <= hh->Left) {
        r = ((char **)hh->Top) + 1;
        hh->Top = ((char *)hh->Top) + len;
        hh->Left -= len;
    }

    return (void *)r;
}



void *
PlincAllocHeap(PlincHeap *h, size_t len)
{
    PlincHeapHeader *hh = h->HeapHeader;
    char **r = NULL;

    len = PLINC_ROUND(len);
   
    if (len <= hh->Left) {
        r = ((char **)hh->Top);
        hh->Top = ((char *)hh->Top) + len;
        hh->Left -= len;
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
PlincFindName(PlincHeapHeader *hh, char *name, size_t len)
{
    unsigned char *n;
    size_t nlen;

    n = hh->Names;
    while (n) {
        nlen = *n;
        if (nlen == len) {
            if (!memcmp(n+1, name, len)) {
                return n;
            }
        }
   
        n = PLINC_LINK(n);
    }

    return NULL;
}



void *
PlincName(PlincHeap *h, char *name, size_t len)
{
    PlincHeapHeader *hh = h->HeapHeader;
    unsigned char *r = NULL;

    len = min(len, PLINC_MAXNAMELEN);

    r = PlincFindName(hh, name, len);
    if (!r) {
        r = PlincAllocHeapLinked(h, len + 1);
        if (r) {
            unsigned char *cp;
               
            PLINC_LINK(r) = hh->Names;
            hh->Names = r;
            cp = r;
            *cp++ = len;
            memcpy(cp, name, len);
        }
    }

    return r;
}



void *
PlincBorrowMemory(PlincHeap *h, PlincUInt *len)
{
    PlincHeapHeader *hh = h->HeapHeader;
    
    *len = hh->Left;

    return hh->Top;
}



void
PlincBorrowAbort(PlincHeap *h, void *p, PlincUInt len)
{
    /* XXX */
}


void *
PlincBorrowFinalize(PlincHeap *h, void *p, PlincUInt borrowed,
               PlincUInt used)
{
    PlincHeapHeader *hh = h->HeapHeader;
    void *r;
    
    used = PLINC_ROUND(used);
   
    r = hh->Top;
    hh->Top = ((char *)hh->Top) + used;
    hh->Left -= used;

    return r;
}
                

#include <plinc/heap.h>
#include <stdio.h>

int
main(void)
{
    char buf[PLINC_NAME_MAX + 1];
    PlincHeap *h;
    PlincPtr p;
    
    h = PlincNewHeap(262144);
    if (h) {
        while (fgets(buf, sizeof(buf), stdin)) {
            buf[strlen(buf)-1] = '\0';
            fprintf(stderr, "NAME: '%s'\n", buf);

            p = PlincName(h, buf, strlen(buf));
            fprintf(stderr, " PTR: '0x%08lX'\n", p);
        }

        fwrite(h->HeapHeader, 1, h->HeapHeader->Len, stdout);
    }
    
    return 0;
}

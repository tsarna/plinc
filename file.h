#ifndef PLINC_IO_H
#define PLINC_IO_H

#define PLINC_EOF   -1
#define PLINC_IOERR -2


typedef struct _PlincFileOps PlincFileOps;
typedef struct _PlincFile PlincFile;

struct _PlincFileOps {
    int (*read)(PlincFile *f);
    int (*write)(PlincFile *f, int c);
};

struct _PlincFile {
    PlincFileOps    *Ops;
    void            *Ptr;

    /* Other data here in "subclasses" */
};


#endif /* PLINC_IO_H */

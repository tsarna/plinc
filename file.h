#ifndef PLINC_IO_H
#define PLINC_IO_H

#define PLINC_EOF   -1
#define PLINC_IOERR -2


typedef struct _PlincFileOps PlincFileOps;
typedef struct _PlincFile PlincFile;

struct _PlincFileOps {
    int         (*close)(PlincFile *f);
    int         (*flush)(PlincFile *f);
    int         (*reset)(PlincFile *f);
    PlincInt    (*bytesavailable)(PlincFile *f);
    int         (*read)(PlincFile *f);
    PlincInt    (*readstring)(PlincFile *f, char *buf, PlincInt l);
    int         (*unread)(PlincFile *f, int c);
    int         (*write)(PlincFile *f, int c);
    PlincInt    (*writestring)(PlincFile *f, char *buf, PlincInt l);
};

struct _PlincFile {
    PlincFileOps    *Ops;
    void            *Ptr;

    /* Other data here in "subclasses" */
};


extern PlincFileOps plinc_closed_ops;

PlincInt    plinc_io_bytesavailable(PlincFile *f);
PlincInt    plinc_io_readstring(PlincFile *f, char *buf, PlincInt l);
PlincInt    plinc_io_writestring(PlincFile *f, char *buf, PlincInt l);


#endif /* PLINC_IO_H */

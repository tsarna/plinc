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
    PlincInt    (*readline)(PlincFile *f, char *buf, PlincInt *l);
    int         (*unread)(PlincFile *f, int c);
    int         (*write)(PlincFile *f, int c);
    PlincInt    (*writestring)(PlincFile *f, char *buf, PlincInt l);
};


#define PlincClose(f)               ((f)->Ops->close(f))
#define PlincFlush(f)               ((f)->Ops->flush(f))
#define PlincReset(f)               ((f)->Ops->reset(f))
#define PlincBytesAvailable(f)      ((f)->Ops->bytesavailable(f))
#define PlincRead(f)                ((f)->Ops->read(f))
#define PlincReadString(f, buf, l)  ((f)->Ops->readstring((f), (buf), (l)))
#define PlincReadLine(f, buf, l)    ((f)->Ops->readline((f), (buf), (l)))
#define PlincUnRead(f, c)           ((f)->Ops->unread((f), (c)))
#define PlincWrite(f, c)            ((f)->Ops->write((f), (c)))
#define PlincWriteString(f, buf, l) ((f)->Ops->writestring((f), (buf), (l)))


struct _PlincFile {
    const PlincFileOps      *Ops;
    void                    *Ptr;

    /* Other data here in "subclasses" */
};


extern const PlincFileOps plinc_closed_ops;

PlincInt    plinc_io_bytesavailable(PlincFile *f);
PlincInt    plinc_io_readstring(PlincFile *f, char *buf, PlincInt l);
PlincInt    plinc_io_readline(PlincFile *f, char *buf, PlincInt *l);
PlincInt    plinc_io_writestring(PlincFile *f, char *buf, PlincInt l);


#endif /* PLINC_IO_H */

#ifndef PLINC_IO_H
#define PLINC_IO_H

#define PLINC_EOF   -1
#define PLINC_IOERR -2


typedef struct _PlincFileOps PlincFileOps;
typedef struct _PlincFile PlincFile;

struct _PlincFileOps {
    int         (*close)(PlincFile *f);
    int         (*readtoeof)(PlincFile *f);
    int         (*flushout)(PlincFile *f);
    int         (*wpurge)(PlincFile *f);
    int         (*rpurge)(PlincFile *f);
    PlincInt    (*bytesavailable)(PlincFile *f);
    int         (*read)(PlincFile *f);
    PlincInt    (*readstring)(PlincFile *f, char *buf, PlincInt l);
    PlincInt    (*readline)(PlincFile *f, char *buf, PlincInt *l);
    int         (*unread)(PlincFile *f, int c);
    int         (*write)(PlincFile *f, int c);
    PlincInt    (*writestring)(PlincFile *f, char *buf, PlincInt l);
};


#define PlincClose(f)               ((f)->Ops->close(f))
#define PlincReadToEOF(f)           ((f)->Ops->readtoeof(f))
#define PlincFlushOut(f)            ((f)->Ops->flushout(f))
#define PlincRPurge(f)              ((f)->Ops->rpurge(f))
#define PlincWPurge(f)              ((f)->Ops->wpurge(f))
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


#define PLINC_HEXBUFSIZE  32


typedef struct _PlincEHexFile {
    const PlincFileOps     *Ops;
    PlincFile              *File;
    PlincUInt               Flags;
#define PLINC_HEXF_CLOSETARGET      0x01
#define PLINC_HEXF_WITHNLS          0x02
#define PLINC_HEXF_WITHEOD          0x04
    PlincUInt               Len;
    PlincUInt               Col;
    char                    Buf[PLINC_HEXBUFSIZE];
} PlincEHexFile;


typedef struct _PlincDHexFile {
    const PlincFileOps     *Ops;
    PlincFile              *File;
    PlincUInt               Flags;
/*      PLINC_HEXF_CLOSETARGET      above */
#define PLINC_HEXF_UNREAD           0x08
    char                    Unread;
} PlincDHexFile;


void    PlincInitHexEncode(PlincEHexFile *hf, PlincFile *f, PlincUInt flags);
void    PlincInitHexDecode(PlincDHexFile *hf, PlincFile *f, PlincUInt flags);


extern const PlincFileOps plinc_closed_ops;

int         plinc_ioerr_file(PlincFile *f);
PlincInt    plinc_ioerr_rdwrstring(PlincFile *f, char *buf, PlincInt l);
int         plinc_ioerr_unreadwrite(PlincFile *f, int c);
PlincInt    plinc_ioerr_readline(PlincFile *f, char *buf, PlincInt *l);

int         plinc_io_readeof(PlincFile *f);
int         plinc_io_flushops(PlincFile *f);
PlincInt    plinc_io_bytesavailable(PlincFile *f);
PlincInt    plinc_io_readstring(PlincFile *f, char *buf, PlincInt l);
PlincInt    plinc_io_readline(PlincFile *f, char *buf, PlincInt *l);
PlincInt    plinc_io_writestring(PlincFile *f, char *buf, PlincInt l);

int         plinc_hex_flushout(PlincFile *f);


#endif /* PLINC_IO_H */

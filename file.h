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


#define PLINC_ENCBUFSIZE  32


typedef struct _PlincEncFile {
    const PlincFileOps     *Ops;
    PlincFile              *File;
    PlincUInt               Flags;
#define PLINC_ENCF_CLOSETARGET      0x01
#define PLINC_ENCF_WITHNLS          0x02
#define PLINC_ENCF_WITHEOD          0x04
#define PLINC_ENCF_WITHBOD          0x08
    PlincUInt               Len;
    PlincUInt               Count;
    char                    Buf[PLINC_ENCBUFSIZE];
} PlincEncFile;


typedef struct _PlincDecodeFile {
    const PlincFileOps     *Ops;
    PlincFile              *File;
    PlincUInt               Flags;
#define PLINC_DECF_CLOSETARGET      0x01
#define PLINC_DECF_UNREAD           0x02
#define PLINC_DECF_WITHEOD          0x04
    PlincUInt               Count;
    int                     Unread;
} PlincDecodeFile;


typedef struct _PlincStrFile {
    const PlincFileOps      *Ops;
    PlincVal                *Obj;
    char                    *Str;
    PlincUInt                Cur;
    PlincUInt                Len;
} PlincStrFile;


void    PlincInitHexEncode(PlincEncFile *hf, PlincFile *f, PlincUInt flags);
void    PlincInitHexDecode(PlincDecodeFile *hf, PlincFile *f, PlincUInt flags);
void    PlincInitPStrEncode(PlincEncFile *hf, PlincFile *f, PlincUInt flags);
void    PlincInitPStrDecode(PlincDecodeFile *hf, PlincFile *f, PlincUInt flags);

void    PlincInitStrFile(PlincStrFile *sf, PlincVal *v);

void   *PlincGetToken(PlincInterp *i, PlincFile *f, PlincVal *val);



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

int         plinc_enc_flushout(PlincFile *f);
int         plinc_dec_close(PlincFile *f);
int         plinc_dec_unread(PlincFile *f, int c);


#endif /* PLINC_IO_H */

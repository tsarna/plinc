#include <plinc/interp.h>
#include <plinc/file.h>

#include <string.h>


static const char hexdigits[] = "0123456789abcdef";



static int
hex_close(PlincFile *f)
{
    PlincHexFile *hf = (PlincHexFile *)f;
    int r = 0;

    if (hf->Flags & PLINC_HEXF_WITHEOD) {
        if ((hf->Len+1) >= PLINC_HEXBUFSIZE) {
            r = plinc_hex_flushout(f);
        }
        hf->Buf[hf->Len++] = '>';
    }

    r = r || plinc_hex_flushout(f);

    if (hf->Flags & PLINC_HEXF_CLOSETARGET) {
        r = r || PlincClose(hf->File);
    }
        
    hf->Ops = &plinc_closed_ops;
    hf->File = NULL;
    hf->Len = 0;
    
    return r;
}



int
plinc_hex_flushout(PlincFile *f)
{
    PlincHexFile *hf = (PlincHexFile *)f;
    int r = 0;

    if (hf->Len) {
        r = PlincWriteString(hf->File, hf->Buf, hf->Len);
        if (r < hf->Len) {
            r = PLINC_IOERR;
        }
        hf->Len = 0;
    }
    
    return r;
}



static int
hex_write(PlincFile *f, int c)
{
    PlincHexFile *hf = (PlincHexFile *)f;
    int r;

    if ((hf->Len + 2) >= PLINC_HEXBUFSIZE) {
        r = plinc_hex_flushout(f);
        if (r) {
            return r;
        }
    }

    hf->Buf[hf->Len++] = hexdigits[(c >> 4) & 0xF];
    hf->Buf[hf->Len++] = hexdigits[c & 0xF];
    hf->Col += 2;
    
    if ((hf->Flags & PLINC_HEXF_WITHNLS) && (hf->Col >= 78)) {
        if ((hf->Len+1) >= PLINC_HEXBUFSIZE) {
            r = plinc_hex_flushout(f);
            if (r) {
                return r;
            }
        }
        hf->Col = 0;
        hf->Buf[hf->Len++] = '\n';
    }
    
    return 0;
}



const PlincFileOps hex_e_ops = {
    hex_close,                  /* close            */
    plinc_io_flushops,          /* readeof          */
    plinc_hex_flushout,         /* flushout         */
    plinc_io_flushops,          /* rpurge           */
    plinc_io_flushops,          /* wpurge           */
    plinc_io_bytesavailable,    /* bytesavailable   */
    plinc_ioerr_file,           /* read             */
    plinc_ioerr_rdwrstring,     /* readstring       */
    plinc_ioerr_readline,       /* readline         */
    plinc_ioerr_unreadwrite,    /* unread           */
    hex_write,                  /* write            */
    plinc_io_writestring,       /* writestring      */
};



void
PlincInitHexEncode(PlincHexFile *hf, PlincFile *f, PlincUInt flags)
{
    hf->Ops = &hex_e_ops;
    hf->File = f;
    hf->Flags = flags;
    hf->Len = hf->Col = 0;
}

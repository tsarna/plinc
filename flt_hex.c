#include <plinc/interp.h>
#include <plinc/file.h>

#include <string.h>


static const char hexdigits[] = "0123456789abcdef";



static int
hex_close(PlincFile *f)
{
    PlincEncFile *hf = (PlincEncFile *)f;
    int r = 0;

    if (hf->Flags & PLINC_ENCF_WITHEOD) {
        if ((hf->Len+1) >= PLINC_ENCBUFSIZE) {
            r = plinc_enc_flushout(f);
        }
        hf->Buf[hf->Len++] = '>';
    }

    r = r || plinc_enc_flushout(f);

    if (hf->Flags & PLINC_ENCF_CLOSETARGET) {
        r = r || PlincClose(hf->File);
    }
        
    hf->Ops = &plinc_closed_ops;
    hf->File = NULL;
    hf->Len = 0;
    
    return r;
}



int
plinc_dec_close(PlincFile *f)
{
    PlincDecodeFile *hf = (PlincDecodeFile *)f;
    int r = 0;

    if (hf->Flags & PLINC_DECF_CLOSETARGET) {
        r = r || PlincClose(hf->File);
    }
        
    hf->Ops = &plinc_closed_ops;
    hf->File = NULL;

    return r;
}



int
plinc_enc_flushout(PlincFile *f)
{
    PlincEncFile *hf = (PlincEncFile *)f;
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
hex_read(PlincFile *f)
{
    PlincDecodeFile *hf = (PlincDecodeFile *)f;
    int n, c, r = 0, exit;

    if (hf->Flags & PLINC_DECF_UNREAD) {
        hf->Flags &= ~PLINC_DECF_UNREAD;
        return hf->Unread;
    } else {
        for (n = 0; n < 2; n++) {
            do {
                c = PlincRead(hf->File);
                exit = FALSE;
                switch (c) {
                case PLINC_IOERR:
                    return c;
                
                case PLINC_EOF:
                    if (n) {
                        return r;
                    } else {
                        return c;
                    }

                case ' ':   case '\t':  case '\r':
                case '\n':  case '\f':  case '\0':
                    continue;
                
                case '0':   case '1':   case '2':   case '3':   case '4':
                case '5':   case '6':   case '7':   case '8':   case '9':
                    r |= c - '0';
                    exit = TRUE;
                    break;
                
                case 'a':   case 'b':   case 'c':
                case 'd':   case 'e':   case 'f':
                    r |= c - 'a' + 10;
                    exit = TRUE;
                    break;
                
                case 'A':   case 'B':   case 'C':
                case 'D':   case 'E':   case 'F':
                    r |= c - 'A' + 10;
                    exit = TRUE;
                    break;
                
                case '>':
                    if (hf->Flags & PLINC_DECF_WITHEOD) {
                        if (n) {
                            hf->Flags |= PLINC_DECF_UNREAD;
                            hf->Unread = PLINC_EOF;
                            return r;
                        } else {
                            return PLINC_EOF;
                        }
                    } else {
                        continue;
                    }
                    
                default:
                    if (hf->Flags & PLINC_DECF_WITHEOD) {
                        return PLINC_IOERR;
                    } else {
                        continue;
                    }
                }
                if (!n) {
                    r <<= 4;
                }
            } while (!exit);
        }
        
        return r;
    }
}



int
plinc_dec_unread(PlincFile *f, int c)
{
    PlincDecodeFile *hf = (PlincDecodeFile *)f;

    if (hf->Flags & PLINC_DECF_UNREAD) {
        return PLINC_IOERR;
    } else {
        hf->Unread = c;
        hf->Flags |= PLINC_DECF_UNREAD;

        return 0;
    }
}



static int
hex_write(PlincFile *f, int c)
{
    PlincEncFile *hf = (PlincEncFile *)f;
    int r;

    if ((hf->Len + 2) >= PLINC_ENCBUFSIZE) {
        r = plinc_enc_flushout(f);
        if (r) {
            return r;
        }
    }

    hf->Buf[hf->Len++] = hexdigits[(c >> 4) & 0xF];
    hf->Buf[hf->Len++] = hexdigits[c & 0xF];
    hf->Count += 2;
    
    if ((hf->Flags & PLINC_ENCF_WITHNLS) && (hf->Count >= 78)) {
        if ((hf->Len+1) >= PLINC_ENCBUFSIZE) {
            r = plinc_enc_flushout(f);
            if (r) {
                return r;
            }
        }
        hf->Count = 0;
        hf->Buf[hf->Len++] = '\n';
    }
    
    return 0;
}



static const PlincFileOps hex_e_ops = {
    hex_close,                  /* close            */
    plinc_io_flushops,          /* readeof          */
    plinc_enc_flushout,         /* flushout         */
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



static const PlincFileOps hex_d_ops = {
    plinc_dec_close,            /* close            */
    plinc_io_readeof,           /* readeof          */
    plinc_io_flushops,          /* flushout         */
    plinc_io_flushops,          /* rpurge           */
    plinc_io_flushops,          /* wpurge           */
    plinc_io_bytesavailable,    /* bytesavailable   */
    hex_read,                   /* read             */
    plinc_io_readstring,        /* readstring       */
    plinc_io_readline,          /* readline         */
    plinc_dec_unread,           /* unread           */
    plinc_ioerr_unreadwrite,    /* write            */
    plinc_ioerr_rdwrstring,     /* writestring      */
};



void
PlincInitHexEncode(PlincEncFile *hf, PlincFile *f, PlincUInt flags)
{
    hf->Ops = &hex_e_ops;
    hf->File = f;
    hf->Flags = flags;
    hf->Len = hf->Count = 0;
}



void
PlincInitHexDecode(PlincDecodeFile *hf, PlincFile *f, PlincUInt flags)
{
    hf->Ops = &hex_d_ops;
    hf->File = f;
    hf->Flags = flags;
}


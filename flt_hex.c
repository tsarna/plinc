#include <plinc/interp.h>
#include <plinc/file.h>

#include <string.h>


static const char hexdigits[] = "0123456789abcdef";



static int
hex_e_close(PlincFile *f)
{
    PlincEHexFile *hf = (PlincEHexFile *)f;
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



static int
hex_d_close(PlincFile *f)
{
    PlincDHexFile *hf = (PlincDHexFile *)f;
    int r = 0;

    if (hf->Flags & PLINC_HEXF_CLOSETARGET) {
        r = r || PlincClose(hf->File);
    }
        
    hf->Ops = &plinc_closed_ops;
    hf->File = NULL;

    return r;
}



int
plinc_hex_flushout(PlincFile *f)
{
    PlincEHexFile *hf = (PlincEHexFile *)f;
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
    PlincDHexFile *hf = (PlincDHexFile *)f;
    int n, c, r = 0, exit;

    if (hf->Flags & PLINC_HEXF_UNREAD) {
        hf->Flags &= ~PLINC_HEXF_UNREAD;
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
                    if (hf->Flags & PLINC_HEXF_WITHEOD) {
                        return r;
                    } else {
                        continue;
                    }
                    
                default:
                    if (hf->Flags & PLINC_HEXF_WITHEOD) {
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



static int
hex_unread(PlincFile *f, int c)
{
    PlincDHexFile *hf = (PlincDHexFile *)f;

    if (hf->Flags & PLINC_HEXF_UNREAD) {
        return PLINC_IOERR;
    } else {
        hf->Unread = c;
        hf->Flags |= PLINC_HEXF_UNREAD;

        return 0;
    }
}



static int
hex_write(PlincFile *f, int c)
{
    PlincEHexFile *hf = (PlincEHexFile *)f;
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



static const PlincFileOps hex_e_ops = {
    hex_e_close,                /* close            */
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



static const PlincFileOps hex_d_ops = {
    hex_d_close,                /* close            */
    plinc_io_readeof,           /* readeof          */
    plinc_io_flushops,          /* flushout         */
    plinc_io_flushops,          /* rpurge           */
    plinc_io_flushops,          /* wpurge           */
    plinc_io_bytesavailable,    /* bytesavailable   */
    hex_read,                   /* read             */
    plinc_io_readstring,        /* readstring       */
    plinc_io_readline,          /* readline         */
    hex_unread,                 /* unread           */
    plinc_ioerr_unreadwrite,    /* write            */
    plinc_ioerr_rdwrstring,     /* writestring      */
};



void
PlincInitHexEncode(PlincEHexFile *hf, PlincFile *f, PlincUInt flags)
{
    hf->Ops = &hex_e_ops;
    hf->File = f;
    hf->Flags = flags;
    hf->Len = hf->Col = 0;
}



void
PlincInitHexDecode(PlincDHexFile *hf, PlincFile *f, PlincUInt flags)
{
    hf->Ops = &hex_d_ops;
    hf->File = f;
    hf->Flags = flags;
}


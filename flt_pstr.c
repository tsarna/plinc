#include <plinc/interp.h>
#include <plinc/file.h>

#include <string.h>


static int
pstr_e_close(PlincFile *f)
{
    PlincEncFile *ef = (PlincEncFile *)f;
    int r = 0;

    if ((ef->Len+1) >= PLINC_ENCBUFSIZE) {
        r = plinc_enc_flushout(f);
    }
    ef->Buf[ef->Len++] = ')';

    r = r || plinc_enc_flushout(f);

    if (ef->Flags & PLINC_ENCF_CLOSETARGET) {
        r = r || PlincClose(ef->File);
    }
        
    ef->Ops = &plinc_closed_ops;
    ef->File = NULL;
    ef->Len = 0;
    
    return r;
}



#if 0
static int
hex_d_close(PlincFile *f)
{
    PlincDHexFile *ef = (PlincDHexFile *)f;
    int r = 0;

    if (ef->Flags & PLINC_HEXF_CLOSETARGET) {
        r = r || PlincClose(ef->File);
    }
        
    ef->Ops = &plinc_closed_ops;
    ef->File = NULL;

    return r;
}



static int
hex_read(PlincFile *f)
{
    PlincDHexFile *ef = (PlincDHexFile *)f;
    int n, c, r = 0, exit;

    if (ef->Flags & PLINC_HEXF_UNREAD) {
        ef->Flags &= ~PLINC_HEXF_UNREAD;
        return ef->Unread;
    } else {
        for (n = 0; n < 2; n++) {
            do {
                c = PlincRead(ef->File);
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
                    if (ef->Flags & PLINC_HEXF_WITHEOD) {
                        return r;
                    } else {
                        continue;
                    }
                    
                default:
                    if (ef->Flags & PLINC_HEXF_WITHEOD) {
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
    PlincDHexFile *ef = (PlincDHexFile *)f;

    if (ef->Flags & PLINC_HEXF_UNREAD) {
        return PLINC_IOERR;
    } else {
        ef->Unread = c;
        ef->Flags |= PLINC_HEXF_UNREAD;

        return 0;
    }
}
#endif



static int
pstr_write(PlincFile *f, int c)
{
    PlincEncFile *ef = (PlincEncFile *)f;
    int r;

    if ((c < 0x20) || (c > 0x7f) || (c == '\\')) {
        if ((ef->Len + 4) >= PLINC_ENCBUFSIZE) {
            r = plinc_enc_flushout(f);
            if (r) {
                return r;
            }
        }
        ef->Buf[ef->Len++] = '\\';
        
        if (c == '\n') {
            ef->Buf[ef->Len++] = 'n';
        } else if (c == '\r') {
            ef->Buf[ef->Len++] = 'r';
        } else if (c == '\t') {
            ef->Buf[ef->Len++] = 't';
        } else if (c == '\b') {
            ef->Buf[ef->Len++] = 'b';
        } else if (c == '\0') {
            ef->Buf[ef->Len++] = '0';
        } else if (c == '\\') {
            ef->Buf[ef->Len++] = '\\';
        } else {
            ef->Buf[ef->Len++] = '0' + ((c >> 6) & 0x3);
            ef->Buf[ef->Len++] = '0' + ((c >> 3) & 0x7);
            ef->Buf[ef->Len++] = '0' + (c & 0x7);
        }
    } else {
        if (c == ')') {
            ef->Count--;
            if (ef->Count < 1) {
                if ((ef->Len + 4) >= PLINC_ENCBUFSIZE) {
                    r = plinc_enc_flushout(f);
                    if (r) {
                        return r;
                    }
                }
                ef->Buf[ef->Len++] = '\\';
            }
        } else if (c == '(') {
            ef->Count++;
        }
        
        if ((ef->Len + 1) >= PLINC_ENCBUFSIZE) {
            r = plinc_enc_flushout(f);
            if (r) {
                return r;
            }
        }

        ef->Buf[ef->Len++] = c;
    }

    return 0;
}



static const PlincFileOps pstr_e_ops = {
    pstr_e_close,               /* close            */
    plinc_io_flushops,          /* readeof          */
    plinc_enc_flushout,         /* flushout         */
    plinc_io_flushops,          /* rpurge           */
    plinc_io_flushops,          /* wpurge           */
    plinc_io_bytesavailable,    /* bytesavailable   */
    plinc_ioerr_file,           /* read             */
    plinc_ioerr_rdwrstring,     /* readstring       */
    plinc_ioerr_readline,       /* readline         */
    plinc_ioerr_unreadwrite,    /* unread           */
    pstr_write,                 /* write            */
    plinc_io_writestring,       /* writestring      */
};



#if 0
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
#endif



void
PlincInitPStrEncode(PlincEncFile *ef, PlincFile *f, PlincUInt flags)
{
    ef->Ops = &pstr_e_ops;
    ef->File = f;
    ef->Flags = flags;
    ef->Len = 0;
    ef->Count = 1;

    if (flags & PLINC_ENCF_WITHBOD) {
        ef->Buf[ef->Len++] = '(';
    }
}



#if 0
void
PlincInitHexDecode(PlincDHexFile *ef, PlincFile *f, PlincUInt flags)
{
    ef->Ops = &hex_d_ops;
    ef->File = f;
    ef->Flags = flags;
}
#endif


#include <plinc/interp.h>
#include <plinc/file.h>

#include <string.h>


static int
pstr_close(PlincFile *f)
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



static int
pstr_read(PlincFile *f)
{
    PlincDecodeFile *ef = (PlincDecodeFile *)f;
    int c, d;

    if (ef->Flags & PLINC_DECF_UNREAD) {
        ef->Flags &= ~PLINC_DECF_UNREAD;
        return ef->Unread;
    } else {
again:
        c = PlincRead(ef->File);
        switch (c) {
        case PLINC_IOERR:
        case PLINC_EOF:
            return PLINC_IOERR;
            
        case '(':
            ef->Count++;
            return c;

        case ')':
            ef->Count--;
            if (ef->Count) {
                return c;
            } else {
                return PLINC_EOF;
            }

        case '\r':
            c = PlincRead(ef->File);
            switch (c) {
            case PLINC_IOERR:
            case PLINC_EOF:
                return PLINC_IOERR;

            case '\n':
                return c;
            
            default:
                c = PlincUnRead(ef->File, c);
                if (c) {
                    return c;
                } else {
                    return '\n';
                }
            }

        case '\\':
            c = PlincRead(ef->File);
            switch (c) {
            case PLINC_IOERR:
            case PLINC_EOF:
                return PLINC_IOERR;
                
            case 'n':
                return '\n';
            
            case 'r':
                return '\r';
             
            case 't':
                return '\t';
             
            case 'b':
                return '\b';
             
            case 'f':
                return '\f';
             
            case '\\':
                return '\\';
             
            case '(':
                return '(';
             
            case ')':
                return ')';
            
            case '\n':
                goto again;
            
            case '\r':
                c = PlincRead(ef->File);
                switch (c) {
                case PLINC_IOERR:
                case PLINC_EOF:
                    return PLINC_IOERR;

                case '\n':
                    goto again;
                
                default:
                    c = PlincUnRead(ef->File, c);
                    if (c) {
                        return c;
                    } else {
                        goto again;
                    }
                }
                
            case '0':   case '1':   case '2':   case '3':
            case '4':   case '5':   case '6':   case '7':
                d = c - '0';
                c = PlincRead(ef->File);
                switch (c) {
                case PLINC_IOERR:
                case PLINC_EOF:
                    return PLINC_IOERR;

                case '0':   case '1':   case '2':   case '3':
                case '4':   case '5':   case '6':   case '7':
                    d <<= 3;
                    d |= c - '0';
                    c = PlincRead(ef->File);
                    switch (c) {
                    case PLINC_IOERR:
                    case PLINC_EOF:
                        return PLINC_IOERR;
    
                    case '0':   case '1':   case '2':   case '3':
                    case '4':   case '5':   case '6':   case '7':
                        d <<= 3;
                        d |= c - '0';
                        return d & 0xFF;
                
                    default:
                        c = PlincUnRead(ef->File, c);
                        if (c) {
                            return c;
                        } else {
                            return d & 0xFF;
                        }
                    }
                    break;
                
                default:
                    c = PlincUnRead(ef->File, c);
                    if (c) {
                        return c;
                    } else {
                        return d & 0xFF;
                    }
                }
                break;
               
            default:
                return c;
            }

        default:
            return c;
        }
    }
}



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
    pstr_close,                 /* close            */
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



static const PlincFileOps pstr_d_ops = {
    plinc_dec_close,            /* close            */
    plinc_io_readeof,           /* readeof          */
    plinc_io_flushops,          /* flushout         */
    plinc_io_flushops,          /* rpurge           */
    plinc_io_flushops,          /* wpurge           */
    plinc_io_bytesavailable,    /* bytesavailable   */
    pstr_read,                  /* read             */
    plinc_io_readstring,        /* readstring       */
    plinc_io_readline,          /* readline         */
    plinc_dec_unread,           /* unread           */
    plinc_ioerr_unreadwrite,    /* write            */
    plinc_ioerr_rdwrstring,     /* writestring      */
};



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



void
PlincInitPStrDecode(PlincDecodeFile *ef, PlincFile *f, PlincUInt flags)
{
    ef->Ops = &pstr_d_ops;
    ef->File = f;
    ef->Flags = flags;
    ef->Count = 1;
}

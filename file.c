#include <plinc/interp.h>
#include <plinc/file.h>

#include <stdlib.h>
#include <string.h>


static const PlincFile closedfile;



static void *
op_file(PlincInterp *i)
{
    PlincVal *v1, *v2;
    char mode = '\0';
        
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);

        if (!PLINC_IS_STRING(*v1) || !PLINC_IS_STRING(*v2)) {
            return i->typecheck;
        } else if (!PLINC_CAN_READ(*v1) || !PLINC_CAN_READ(*v2)) {
            return i->invalidaccess;
        } else {
            if (PLINC_SIZE(*v2) == 1) {
                mode = *(char *)(v2->Val.Ptr);
            }

            if ((PLINC_SIZE(*v1) == 6) && !memcmp(v1->Val.Ptr, "%stdin", 6)) {
                if (mode == 'r') {
                    PLINC_OPPOP(i);
                    PLINC_OPPOP(i);
                    PLINC_OPPUSH(i, i->StdIn);
                } else {
                    return i->invalidfileaccess;
                }
            } else if ((PLINC_SIZE(*v1) == 7) && !memcmp(v1->Val.Ptr, "%stdout", 7)) {
                if (mode == 'w') {
                    PLINC_OPPOP(i);
                    PLINC_OPPOP(i);
                    PLINC_OPPUSH(i, i->StdOut);
                } else {
                    return i->invalidfileaccess;
                }
            } else {
                return i->undefinedfilename; /* XXX */
            }
        }
    }        
    
    return NULL;
}



static void *
op_closefile(PlincInterp *i)
{
    PlincFile *f;
    PlincVal *v;
        
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);

        if (PLINC_TYPE(*v) != PLINC_TYPE_FILE){
            return i->typecheck;
        } else {
            f = (PlincFile *)(v->Val.Ptr);
            if ((PlincClose(f)) == PLINC_IOERR) {
                return i->ioerror;
            }
        }
    }
    
    return NULL;
}



static void *
op_read(PlincInterp *i)
{
    PlincVal *v, nv;
    PlincFile *f;
    int c;
     
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) == PLINC_TYPE_FILE) {
            if (!PLINC_CAN_READ(*v)) {
                return i->invalidaccess;
            } else {
                f = (PlincFile *)(v->Val.Ptr);
                c = PlincRead(f);
                if (c == PLINC_IOERR) {
                    return i->ioerror;
                } else if (c == PLINC_EOF) {
                    nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
                    nv.Val.Int = FALSE;

                    PLINC_OPPOP(i);
                    PLINC_OPPUSH(i, nv);
                } else {
                    nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
                    nv.Val.Int = c;

                    PLINC_OPPOP(i);
                    PLINC_OPPUSH(i, nv);

                    nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
                    nv.Val.Int = TRUE;

                    PLINC_OPPUSH(i, nv);
                }
            }
        } else {
            return i->typecheck;
        }
    }

    return NULL;
}



static void *
op_readstring(PlincInterp *i)
{
    PlincVal *v1, *v2, nv;
    PlincFile *f;
    PlincInt r, s;
     
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);
        
        if ((PLINC_TYPE(*v1) != PLINC_TYPE_FILE) 
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_STRING)) {
            return i->typecheck;
        } else if (!PLINC_CAN_READ(*v1) || !PLINC_CAN_WRITE(*v2)) {
            return i->invalidaccess;
        } else {
            f = (PlincFile *)(v1->Val.Ptr);
            s = PLINC_SIZE(*v2);
            r = PlincReadString(f, v2->Val.Ptr, s);
            if (r == PLINC_IOERR) {
                return i->ioerror;
            } else {
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);

                nv = *v2;
                nv.Flags &= ~PLINC_SIZE_MASK;
                nv.Flags |= r;
                PLINC_OPPUSH(i, nv);

                nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
                nv.Val.Int = (r == s) ? TRUE : FALSE;
                PLINC_OPPUSH(i, nv);
            }
        }
    }

    return NULL;
}



static void *
op_readhexstring(PlincInterp *i)
{
    PlincVal *v1, *v2, nv;
    PlincDecodeFile hf;
    PlincFile *f;
    PlincInt r, s;
     
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);
        
        if ((PLINC_TYPE(*v1) != PLINC_TYPE_FILE) 
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_STRING)) {
            return i->typecheck;
        } else if (!PLINC_CAN_READ(*v1) || !PLINC_CAN_WRITE(*v2)) {
            return i->invalidaccess;
        } else {
            f = (PlincFile *)(v1->Val.Ptr);
            PlincInitHexDecode(&hf, f, 0);
            s = PLINC_SIZE(*v2);
            r = PlincReadString((PlincFile *)(void *)(&hf), v2->Val.Ptr, s);
            if (r == PLINC_IOERR) {
                return i->ioerror;
            } else {
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);

                nv = *v2;
                nv.Flags &= ~PLINC_SIZE_MASK;
                nv.Flags |= r;
                PLINC_OPPUSH(i, nv);

                nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
                nv.Val.Int = (r == s) ? TRUE : FALSE;
                PLINC_OPPUSH(i, nv);
            }
        }
    }

    return NULL;
}



static void *
op_readline(PlincInterp *i)
{
    PlincVal *v1, *v2, nv;
    PlincFile *f;
    PlincInt r, s;
     
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);
        
        if ((PLINC_TYPE(*v1) != PLINC_TYPE_FILE) 
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_STRING)) {
            return i->typecheck;
        } else if (!PLINC_CAN_READ(*v1) || !PLINC_CAN_WRITE(*v2)) {
            return i->invalidaccess;
        } else {
            f = (PlincFile *)(v1->Val.Ptr);
            s = PLINC_SIZE(*v2);
            r = PlincReadLine(f, v2->Val.Ptr, &s);
            if (r == PLINC_IOERR) {
                return i->ioerror;
            } else if (r == FALSE) {
                return i->rangecheck;
            } else {
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);

                nv = *v2;
                nv.Flags &= ~PLINC_SIZE_MASK;
                nv.Flags |= s;
                PLINC_OPPUSH(i, nv);

                nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
                nv.Val.Int = (r == TRUE) ? TRUE : FALSE;
                PLINC_OPPUSH(i, nv);
            }
        }
    }

    return NULL;
}



static void *
op_write(PlincInterp *i)
{
    PlincVal *v1, *v2;
    PlincFile *f;
    int c;
     
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);
        
        if ((PLINC_TYPE(*v1) != PLINC_TYPE_FILE)
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_INT)) {
            return i->typecheck;
        } else if (!PLINC_CAN_WRITE(*v1)) {
            return i->invalidaccess;
        } else {
            f = (PlincFile *)(v1->Val.Ptr);
            c = PlincWrite(f, v2->Val.Int);
            if (c == PLINC_IOERR) {
                return i->ioerror;
            } else {
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
            }
        }
    }

    return NULL;
}



static void *
op_writestring(PlincInterp *i)
{
    PlincVal *v1, *v2;
    PlincFile *f;
    int c;
     
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);
        
        if (!PLINC_IS_FILE(*v1) || !PLINC_IS_STRING(*v2)) {
            return i->typecheck;
        } else if ((!PLINC_CAN_WRITE(*v1)) || (!PLINC_CAN_READ(*v2))) {
            return i->invalidaccess;
        } else {
            f = (PlincFile *)(v1->Val.Ptr);
            c = PlincWriteString(f, v2->Val.Ptr, PLINC_SIZE(*v2));
            if ((c == PLINC_IOERR) || (c < PLINC_SIZE(*v2))) {
                return i->ioerror;
            }

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
        }
    }

    return NULL;
}



static void *
op_writehexstring(PlincInterp *i)
{
    PlincVal *v1, *v2;
    PlincEncFile hf;
    PlincFile *f;
    int r;
     
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);
        
        if (!PLINC_IS_FILE(*v1) || !PLINC_IS_STRING(*v2)) {
            return i->typecheck;
        } else if ((!PLINC_CAN_WRITE(*v1)) || (!PLINC_CAN_READ(*v2))) {
            return i->invalidaccess;
        } else {
            f = (PlincFile *)(v1->Val.Ptr);

            PlincInitHexEncode(&hf, f, 0);
            r = PlincWriteString((PlincFile *)&hf, v2->Val.Ptr, PLINC_SIZE(*v2));
            r = (r < PLINC_SIZE(*v2)) ? PLINC_IOERR : \
                plinc_enc_flushout((PlincFile *)&hf);

            if (r == PLINC_IOERR) {
                return i->ioerror;
            } else {
                PLINC_OPPOP(i);
                PLINC_OPPOP(i);
            }
        }
    }

    return NULL;
}



static void *
op_bytesavailable(PlincInterp *i)
{
    PlincVal *v, nv;
    PlincFile *f;
    PlincInt c;
     
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) != PLINC_TYPE_FILE) {
            return i->typecheck;
        } else if (!PLINC_CAN_READ(*v)) {
            return i->invalidaccess;
        } else {
            f = (PlincFile *)(v->Val.Ptr);
            c = PlincBytesAvailable(f);

            if (c == PLINC_IOERR) {
                return i->ioerror;
            }
            
            nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_INT;
            nv.Val.Int = c;

            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, nv);
        }
    }

    return NULL;
}



static void *
op_flush(PlincInterp *i)
{
    PlincFile *f;
    
    f = (PlincFile *)(i->StdOut.Val.Ptr);

    return ((PlincFlushOut(f)) == PLINC_IOERR) ? i->ioerror : NULL;
}



static void *
op_flushfile(PlincInterp *i)
{
    PlincVal *v;
    PlincFile *f;
    void *r = NULL;
     
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) != PLINC_TYPE_FILE) {
            return i->typecheck;
        } else {
            f = (PlincFile *)(v->Val.Ptr);
            if (PLINC_CAN_WRITE(*v)) {
                r = ((PlincFlushOut(f)) == PLINC_IOERR) ? i->ioerror : NULL;
            } else if (PLINC_CAN_READ(*v)) {
                r = ((PlincReadToEOF(f)) == PLINC_IOERR) ? i->ioerror : NULL;
            }
            if (r) {
                return r;
            }

            PLINC_OPPOP(i);
        }
    }

    return r;
}



static void *
op_resetfile(PlincInterp *i)
{
    PlincVal *v;
    PlincFile *f;
    void *r = NULL;
     
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) != PLINC_TYPE_FILE) {
            return i->typecheck;
        } else {
            f = (PlincFile *)(v->Val.Ptr);
            if (PLINC_CAN_WRITE(*v)) {
                r = ((PlincWPurge(f)) == PLINC_IOERR) ? i->ioerror : NULL;
            }
            if (!r && PLINC_CAN_READ(*v)) {
                r = ((PlincRPurge(f)) == PLINC_IOERR) ? i->ioerror : NULL;
            }
            if (r) {
                return r;
            }

            PLINC_OPPOP(i);
        }
    }

    return r;
}



static void *
op_status(PlincInterp *i)
{
    PlincVal *v, nv;
    PlincFile *f;
     
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) != PLINC_TYPE_FILE) {
            return i->typecheck;
        } else {
            f = (PlincFile *)(v->Val.Ptr);

            nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
            nv.Val.Int = (f->Ops == &plinc_closed_ops) ? FALSE : TRUE;

            PLINC_OPPOP(i);
            PLINC_OPPUSH(i, nv);
        }
    }

    return NULL;
}



static void *
op_currentfile(PlincInterp *i)
{
    PlincVal nv;
    int n;
     
    if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        for (n = 0; n < i->ExecStack.Len; n++) {
            if (PLINC_IS_FILE(PLINC_TOPDOWN(i->ExecStack, n))) {
                PLINC_OPPUSH(i, PLINC_TOPDOWN(i->ExecStack, n));
                return NULL;
            }
        }
        
        nv.Flags = PLINC_TYPE_FILE | PLINC_ATTR_LIT;
        nv.Val.Ptr = (void *)&closedfile;
        PLINC_OPPUSH(i, nv);
    }

    return NULL;
}



static void *
op_echo(PlincInterp *i)
{
    PlincVal *v;
     
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) != PLINC_TYPE_BOOL) {
            return i->typecheck;
        } else {
            if (v->Val.Int) {
                i->Flags |= PLINC_FLAG_ECHO;
            } else {
                i->Flags &= ~PLINC_FLAG_ECHO;
            }

            PLINC_OPPOP(i);
        }
    }

    return NULL;
}



static void *
op_print(PlincInterp *i)
{
    PlincVal *v;
    PlincFile *f;
    int c;
     
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) != PLINC_TYPE_STRING) {
            return i->typecheck;
        } else if (!PLINC_CAN_READ(*v)) {
            return i->invalidaccess;
        } else {
            f = (PlincFile *)(i->StdOut.Val.Ptr);
            c = PlincWriteString(f, v->Val.Ptr, PLINC_SIZE(*v));

            if (c == PLINC_IOERR) {
                return i->ioerror;
            }

            PLINC_OPPOP(i);
        }
    }

    return NULL;
}



static const PlincOp ops[] = {
    {op_file,           "file"},
    {op_closefile,      "closefile"},
    {op_read,           "read"},
    {op_readstring,     "readstring"},
    {op_readhexstring,  "readhexstring"},
    {op_readline,       "readline"},
    {op_write,          "write"},
    {op_writestring,    "writestring"},
    {op_writehexstring, "writehexstring"},
    {op_bytesavailable, "bytesavailable"},
    {op_flush,          "flush"},
    {op_flushfile,      "flushfile"},
    {op_resetfile,      "resetfile"},
    {op_status,         "status"},
    {op_currentfile,    "currentfile"},
    {op_print,          "print"},
    {op_echo,           "echo"},

    {NULL,              NULL}
};



void
PlincInitFileOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}



int
plinc_ioerr_file(PlincFile *f)
{
    /* same call signature for read, close, flush, reset */
    
    return PLINC_IOERR;
}



int
plinc_ioerr_unreadwrite(PlincFile *f, int c)
{
    return PLINC_IOERR;
}



PlincInt
plinc_ioerr_rdwrstring(PlincFile *f, char *buf, PlincInt l)
{
    return PLINC_IOERR;
}



PlincInt
plinc_ioerr_readline(PlincFile *f, char *buf, PlincInt *l)
{
    return PLINC_IOERR;
}



/*
 *  default implementation of readtoeof
 */
int
plinc_io_readeof(PlincFile *f)
{
    int c;
    
    do {
        c = PlincRead(f);
    } while (c >= 0);
    
    return (c == PLINC_EOF) ? 0 : c;
}



/*
 *  default no-op implementation of readtoeof/flushout/purge for a file
 */
int
plinc_io_flushops(PlincFile *f)
{
    return 0;
}



/*
 *  default implementation of bytesavailable for a file
 */
PlincInt
plinc_io_bytesavailable(PlincFile *f)
{
    return -1;
}



/*
 *  default readstring implementation for a file that only provides read
 */
PlincInt
plinc_io_readstring(PlincFile *f, char *buf, PlincInt l)
{
    PlincInt r = 0;
    int c;
    
    while (l) {
        c = PlincRead(f);
        if (c == PLINC_IOERR) {
            return PLINC_IOERR;
        } else if (c == PLINC_EOF) {
            return r;
        } else {
            *buf++ = (char)c;
            l--; r++;
        }
    }
    
    return r;
}



/*
 *  default readline implementation for a file that only provides read
 *
 *  l is a pointer to the size of the buffer on input, and
 *  is changed to the ammount actually read on return.
 *
 *  returns: TRUE if read a line, PLINC_EOF if hit EOF before EOL,
 *  PLINC_IOERR on error, FALSE if filled buffer before hitting EOL.
 */
PlincInt
plinc_io_readline(PlincFile *f, char *buf, PlincInt *l)
{
    int c, len;
    
    len = *l;
    *l = 0;
    
    while (len) {
        c = PlincRead(f);
        if (c < 0) { /* IOERR or EOF */
            return c;
        } else {
            if (c == '\n') {
                return TRUE;
            } else if (c == '\r') {
                c = PlincRead(f);
                if (c == PLINC_EOF) {
                    return TRUE; /* CR was EOL */
                } else if (c == PLINC_IOERR) {
                    return c;
                } else if (c == '\n') {
                    return TRUE;
                } else {
                    /* CR was EOL, whatever we got starts the next line */
                    c = PlincUnRead(f, c);
                    if (c) {
                        return c;
                    }
                    return TRUE;
                }
            } else {
                *buf++ = (char)c;
                (*l)++; len--;
            }
        }
    }
    
    return FALSE;   /* filled before EOL */
}



/*
 *  default writestring implementation for a file that only provides write
 */
PlincInt
plinc_io_writestring(PlincFile *f, char *buf, PlincInt l)
{
    PlincInt c, r = 0;
    
    while (l) {
        c = PlincWrite(f, *buf++);
        if (c) {
            return c;
        } else {
            l--; r++;
        }
    }
    
    return r;
}



const PlincFileOps plinc_closed_ops = {
    plinc_ioerr_file,           /* close */
    plinc_ioerr_file,           /* readtoeof */
    plinc_ioerr_file,           /* flushout */
    plinc_ioerr_file,           /* rpurge */
    plinc_ioerr_file,           /* wpurge */
    plinc_io_bytesavailable,    /* bytesavailable */
    plinc_ioerr_file,           /* read  */
    plinc_ioerr_rdwrstring,     /* readstring */
    plinc_ioerr_readline,       /* readline */
    plinc_ioerr_unreadwrite,    /* unread */
    plinc_ioerr_unreadwrite,    /* write */
    plinc_ioerr_rdwrstring,     /* writestring */
};


static const PlincFile closedfile = {
    &plinc_closed_ops,
    NULL
};

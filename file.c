#include <plinc/interp.h>
#include <plinc/file.h>

#include <stdlib.h>
#include <string.h>


#ifndef PLINC_HEX_BUFSIZE
#define PLINC_HEX_BUFSIZE   32
#endif
#define HEXCHUNK ((PLINC_HEX_BUFSIZE)/2)

static const char hexdigits[] = "0123456789abcdef";



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

        if ((PLINC_TYPE(*v1) != PLINC_TYPE_STRING) 
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_STRING)) {
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
            if ((f->Ops->close(f)) == PLINC_IOERR) {
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
                c = f->Ops->read(f);
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
            r = f->Ops->readstring(f, v2->Val.Ptr, s);
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
            r = f->Ops->readline(f, v2->Val.Ptr, &s);
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
            c = f->Ops->write(f, v2->Val.Int);
            if (c == PLINC_IOERR) {
                return i->ioerror;
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
        
        if ((PLINC_TYPE(*v1) != PLINC_TYPE_FILE)
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_STRING)) {
            return i->typecheck;
        } else if ((!PLINC_CAN_WRITE(*v1)) || (!PLINC_CAN_READ(*v2))) {
            return i->invalidaccess;
        } else {
            f = (PlincFile *)(v1->Val.Ptr);
            c = f->Ops->writestring(f, v2->Val.Ptr, PLINC_SIZE(*v2));

            if (c == PLINC_IOERR) {
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
    char buf[PLINC_HEX_BUFSIZE], *p, *s;
    PlincVal *v1, *v2;
    PlincFile *f;
    int c, l, h;
     
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);
        
        if ((PLINC_TYPE(*v1) != PLINC_TYPE_FILE)
        ||  (PLINC_TYPE(*v2) != PLINC_TYPE_STRING)) {
            return i->typecheck;
        } else if ((!PLINC_CAN_WRITE(*v1)) || (!PLINC_CAN_READ(*v2))) {
            return i->invalidaccess;
        } else {
            f = (PlincFile *)(v1->Val.Ptr);
            l = PLINC_SIZE(*v2);
            s = v2->Val.Ptr;
            
            while (l >= HEXCHUNK) {
                h = HEXCHUNK; p = buf;
                while (h) {
                    *p++ = hexdigits[*s >> 4];
                    *p++ = hexdigits[(*s++) & 0xF];
                    h--;
                }
                
                c = f->Ops->writestring(f, buf, PLINC_HEX_BUFSIZE);
                if (c == PLINC_IOERR) {
                    return i->ioerror;
                }
                l -= HEXCHUNK;
            }

            h = 0; p = buf;
            while (l) {
                *p++ = hexdigits[*s >> 4];
                *p++ = hexdigits[(*s++) & 0xF];
                h++; l--;
            }
                
            c = f->Ops->writestring(f, buf, h*2);
            if (c == PLINC_IOERR) {
                return i->ioerror;
            }

            PLINC_OPPOP(i);
            PLINC_OPPOP(i);
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
            c = f->Ops->bytesavailable(f);

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

    return ((f->Ops->flush(f)) == PLINC_IOERR) ? i->ioerror : NULL;
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
            r = ((f->Ops->flush(f)) == PLINC_IOERR) ? i->ioerror : NULL;

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
            r = ((f->Ops->reset(f)) == PLINC_IOERR) ? i->ioerror : NULL;

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
            c = f->Ops->writestring(f, v->Val.Ptr, PLINC_SIZE(*v));

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
    {op_readline,       "readline"},
    {op_write,          "write"},
    {op_writestring,    "writestring"},
    {op_writehexstring, "writehexstring"},
    {op_bytesavailable, "bytesavailable"},
    {op_flush,          "flush"},
    {op_flushfile,      "flushfile"},
    {op_resetfile,      "resetfile"},
    {op_status,         "status"},
    {op_print,          "print"},

    {NULL,              NULL}
};



void
PlincInitFileOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}



static int
closed_file(PlincFile *f)
{
    /* same call signature for read, close, flush, reset */
    
    return PLINC_IOERR;
}



static int
closed_writeorunread(PlincFile *f, int c)
{
    return PLINC_IOERR;
}



static PlincInt
closed_readorwritestring(PlincFile *f, char *buf, PlincInt l)
{
    return PLINC_IOERR;
}



static PlincInt
closed_readline(PlincFile *f, char *buf, PlincInt *l)
{
    return PLINC_IOERR;
}



const PlincFileOps plinc_closed_ops = {
    closed_file,                /* close */
    closed_file,                /* flush */
    closed_file,                /* reset */
    plinc_io_bytesavailable,    /* bytesavailable */
    closed_file,                /* read  */
    closed_readorwritestring,   /* readstring */
    closed_readline,            /* readline */
    closed_writeorunread,       /* unread */
    closed_writeorunread,       /* write */
    closed_readorwritestring,   /* writestring */
};

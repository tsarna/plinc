#include <plinc/interp.h>
#include <plinc/file.h>

#include <string.h>



static int
str_close(PlincFile *f)
{
    PlincStrFile *sf = (PlincStrFile *)f;

    PLINC_DECREF_VAL(*(sf->Obj));

    sf->Ops = &plinc_closed_ops;
    sf->Obj = NULL;
    sf->Str = NULL;
    sf->Len = sf->Cur = 0;
    
    return 0;
}



static int
str_readtoeof(PlincFile *f)
{
    PlincStrFile *sf = (PlincStrFile *)f;

    sf->Cur = sf->Len;

    return 0;
}



static PlincInt
str_bytesavailable(PlincFile *f)
{
    PlincStrFile *sf = (PlincStrFile *)f;

    return sf->Len - sf->Cur;
}



static int
str_read(PlincFile *f)
{
    PlincStrFile *sf = (PlincStrFile *)f;

    if (sf->Cur < sf->Len) {
        return sf->Str[sf->Cur++];
    } else {
        return PLINC_EOF;
    }
}



static int
str_write(PlincFile *f, int c)
{
    PlincStrFile *sf = (PlincStrFile *)f;

    if (sf->Cur < sf->Len) {
        sf->Str[sf->Cur++] = c;
        
        return 0;
    } else {
        return PLINC_IOERR;
    }
}



static PlincInt
str_readstring(PlincFile *f, char *buf, PlincInt l)
{
    PlincStrFile *sf = (PlincStrFile *)f;
    
    l = min(l, (sf->Len - sf->Cur));
    memcpy(buf, &(sf->Str[sf->Cur]), l);
    sf->Cur += l;
    
    return l;
}



static int
str_unread(PlincFile *f, int c)
{
    PlincStrFile *sf = (PlincStrFile *)f;

    if (sf->Cur > 0) {
        sf->Str[--(sf->Cur)] = c;
        
        return 0;
    } else {
        return PLINC_IOERR;
    }
}



static PlincInt
str_writestring(PlincFile *f, char *buf, PlincInt l)
{
    PlincStrFile *sf = (PlincStrFile *)f;
    
    l = min(l, (sf->Len - sf->Cur));
    memcpy(&(sf->Str[sf->Cur]), buf, l);
    sf->Cur += l;
    
    return l;
}



const PlincFileOps str_ops = {
    str_close,
    str_readtoeof,
    plinc_io_flushops,          /* flushout */
    plinc_io_flushops,          /* rpurge */
    plinc_io_flushops,          /* wpurge */
    str_bytesavailable,
    str_read,
    str_readstring,
    plinc_io_readline,
    str_unread,
    str_write,
    str_writestring,
};



void
PlincInitStrFile(PlincStrFile *sf, PlincVal *v)
{
    sf->Ops = &str_ops;
    sf->Obj = v;
    sf->Str = v->Val.Ptr;
    sf->Len = PLINC_SIZE(*v);
    sf->Cur = 0;
}



static void *
op_Xstrfile(PlincInterp *i)     /* XXX for debugging only */
{
    PlincVal *v, nv;
    PlincStrFile *sf;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        if (!PLINC_IS_STRING(*v)) {
            return i->typecheck;
        } else {
            sf = PlincAllocHeap(i->Heap, sizeof(PlincStrFile));
            if (!sf) {
                return i->VMerror;
            } else {
                PLINC_INCREF_VAL(*v);
                PlincInitStrFile(sf, v);

                nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_FILE
                    | (v->Flags & PLINC_ACCESS_MASK);
                nv.Val.Ptr = sf;
                
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, nv);
            }
        }
        return NULL;
    }
}



static const PlincOp ops[] = {
    {op_Xstrfile,   "Xstrfile"},

    {NULL,          NULL}
};



void
PlincInitFltOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}


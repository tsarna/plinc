#include <plinc/interp.h>
#include <plinc/version.h>

#include <string.h>


typedef struct _PlincString PlincString;
struct _PlincString {
    PlincUInt   Flags;
};



void *
PlincNewString(PlincHeap *h, PlincUInt size)
{
    PlincHeapHeader *hh = h->HeapHeader;
    PlincString *r = NULL;

    r = PlincAllocHeap(h, sizeof(PlincString) + size);

    if (r) {
        PLINC_LINK(r) = hh->Objects;
        hh->Objects = r;

        r->Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | size;

        memset(++r, 0, size);
    }

    return r;
}


                
static void *
op_string(PlincInterp *i)
{
    PlincVal *v, nv;
    void *r;
    int j;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) == PLINC_TYPE_INT) {
            j = v->Val.Int;
            if ((j >= 0) && (j < PLINC_MAXLEN)) {
                r = PlincNewString(i->Heap, j);
                if (r) {
                    nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | j;
                    nv.Val.Ptr = r;
                    
                    PLINC_OPPOP(i);
                    PLINC_OPPUSH(i, nv);
                } else {
                    return i->VMerror;
                }
            } else {
                return i->rangecheck;
            }
        } else {
            return i->typecheck;
        }
    }

    return NULL;
}


                
static void *
op_anchorsearch(PlincInterp *i)
{
    PlincVal *v1, *v2, nv;
    PlincUInt32 l1, l2;
    char *p1;
        
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);
        
        if ((PLINC_TYPE(*v1) != PLINC_TYPE_STRING)
        &&  (PLINC_TYPE(*v2) != PLINC_TYPE_STRING)) {
            return i->typecheck;
        } else if (!PLINC_CAN_READ(*v1) || !PLINC_CAN_READ(*v2)){
            return i->invalidaccess;
        } else {
            p1 = v1->Val.Ptr;
            l1 = PLINC_SIZE(*v1);
            l2 = PLINC_SIZE(*v2);

            if (l1 >= l2) {
                if (!memcmp(p1, v2->Val.Ptr, l2)) {
                    PLINC_OPPOP(i);
                    PLINC_OPPOP(i);
                
                    nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | (l1 - l2);
                    nv.Val.Ptr = (p1 + l2);
                    PLINC_OPPUSH(i, nv);

                    nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | (l2);
                    nv.Val.Ptr = p1;
                    PLINC_OPPUSH(i, nv);

                    nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
                    nv.Val.Int = TRUE;
                    PLINC_OPPUSH(i, nv);    

                    return NULL;
                }
            }
            
            PLINC_OPPOP(i);

            nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
            nv.Val.Int = FALSE;
                    
            PLINC_OPPUSH(i, nv);
        }
    }

    return NULL;
}


                
static void *
op_search(PlincInterp *i)
{
    PlincVal *v1, *v2, nv;
    PlincUInt32 l1, l2, ix;
    char *p1, *p2;
        
    if (!PLINC_OPSTACKHAS(i, 2)) {
        return i->stackunderflow;
    } else if (!PLINC_OPSTACKROOM(i, 2)) {
        return i->stackoverflow;
    } else {
        v1 = &PLINC_OPTOPDOWN(i, 1);
        v2 = &PLINC_OPTOPDOWN(i, 0);
        
        if ((PLINC_TYPE(*v1) != PLINC_TYPE_STRING)
        &&  (PLINC_TYPE(*v2) != PLINC_TYPE_STRING)) {
            return i->typecheck;
        } else if (!PLINC_CAN_READ(*v1) || !PLINC_CAN_READ(*v2)){
            return i->invalidaccess;
        } else {
            p1 = v1->Val.Ptr;
            p2 = v2->Val.Ptr;
            l1 = PLINC_SIZE(*v1);
            l2 = PLINC_SIZE(*v2);

            if (l1 >= l2) {
                for (ix = 0; ix <= (l1 - l2); ix++) {
                    if (!memcmp(p1+ix, p2, l2)) {
                        PLINC_OPPOP(i);
                        PLINC_OPPOP(i);
                
                        nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | (l1 - l2 - ix);
                        nv.Val.Ptr = (p1 + ix + l2);
                        PLINC_OPPUSH(i, nv);

                        nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | (l2);
                        nv.Val.Ptr = (p1 + ix);
                        PLINC_OPPUSH(i, nv);

                        nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | (ix);
                        nv.Val.Ptr = p1;
                        PLINC_OPPUSH(i, nv);

                        nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
                        nv.Val.Int = TRUE;
                        PLINC_OPPUSH(i, nv);    

                        return NULL;
                    }
                }                    
            }
            
            PLINC_OPPOP(i);

            nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_BOOL;
            nv.Val.Int = FALSE;
                    
            PLINC_OPPUSH(i, nv);
        }
    }

    return NULL;
}



static void *
op_cvn(PlincInterp *i)
{
    PlincVal *v, nv;
    
    if (!PLINC_OPSTACKHAS(i, 1)) {
        return i->stackunderflow;
    } else {
        v = &PLINC_OPTOPDOWN(i, 0);
        
        if (PLINC_TYPE(*v) != PLINC_TYPE_STRING) {
            return i->typecheck;
        } else {
            nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_NAME;
            nv.Val.Ptr = PlincName(i->Heap, v->Val.Ptr,
                min(PLINC_SIZE(*v), PLINC_MAXNAMELEN));

            if (nv.Val.Ptr) {
                PLINC_OPPOP(i);
                PLINC_OPPUSH(i, nv);
                
                return NULL;
            } else {
                return i->VMerror;
            }
        }
    }
}



static void *
op_version(PlincInterp *i)
{
    PlincVal nv;
    
    if (!PLINC_OPSTACKROOM(i, 1)) {
        return i->stackoverflow;
    } else {
        nv.Flags = PLINC_ATTR_LIT | PLINC_TYPE_STRING | PLINC_ATTR_NOWRITE \
            | strlen(PLINC_VERSION);
        nv.Val.Ptr = PLINC_VERSION;

        PLINC_OPPUSH(i, nv);
    }

    return NULL;
}



static const PlincOp ops[] = {
    {op_string,         "string"},
    {op_anchorsearch,   "anchorsearch"},
    {op_search,         "search"},
    {op_cvn,            "cvn"},
    {op_version,        "version"},

    {NULL,              NULL}
};



void
PlincInitStringOps(PlincInterp *i)
{
    PlincInitOps(i, ops);
}

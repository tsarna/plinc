#ifndef PLINC_TYPES_H
#define PLINC_TYPES_H

/* $Endicor: types.h,v 1.8 1999/01/18 00:54:54 tsarna Exp $ */

#include <sys/types.h>


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#define PLINC_ALIGN         4
#define PLINC_ROUND(x)      (((x) + PLINC_ALIGN - 1) & ~(PLINC_ALIGN - 1))
#define PLINC_LINK(o)       (*(((void **)(o)) - 1))

typedef int32_t     PlincInt;
typedef u_int32_t   PlincUInt;

#define PLINC_ATTR_LIT      0x80000000
#define PLINC_ATTR_NOREAD   0x40000000
#define PLINC_ATTR_NOWRITE  0x20000000
#define PLINC_ATTR_NOEXEC   0x10000000
#define PLINC_ATTR_COWED    0x08000000
#define PLINC_ATTR_DOEXEC   0x04000000
#define PLINC_TYPE_MASK     0x03F00000
#define PLINC_SIZE_MASK     0x000FFFFF

#define PLINC_MAXLEN        (PLINC_SIZE_MASK - 1)

#define PLINC_LIT(x)        ((x).Flags & PLINC_ATTR_LIT)
#define PLINC_EXEC(x)       (!((x).Flags & PLINC_ATTR_LIT))
#define PLINC_DOEXEC(x)     ((x).Flags & PLINC_ATTR_DOEXEC)
#define PLINC_CAN_READ(x)   (!((x).Flags & PLINC_ATTR_NOREAD))
#define PLINC_CAN_WRITE(x)  (!((x).Flags & PLINC_ATTR_NOWRITE))
#define PLINC_CAN_EXEC(x)   (!((x).Flags & PLINC_ATTR_NOEXEC))

#define PLINC_TYPE(x)       ((x).Flags & PLINC_TYPE_MASK)
#define PLINC_SIZE(x)       ((x).Flags & PLINC_SIZE_MASK)

#define PLINC_TYPE_INT      0x00000000
#define PLINC_TYPE_REAL     0x00100000
#define PLINC_TYPE_BOOL     0x00200000
#define PLINC_TYPE_ARRAY    0x00300000
#define PLINC_TYPE_STRING   0x00400000
#define PLINC_TYPE_NAME     0x00500000
#define PLINC_TYPE_DICT     0x00600000
#define PLINC_TYPE_OP       0x00700000
#define PLINC_TYPE_FILE     0x00800000
#define PLINC_TYPE_MARK     0x00900000
#define PLINC_TYPE_NULL     0x00A00000
#define PLINC_TYPE_INVALID  0x00A00000 /* XXX ??? */
#define PLINC_TYPE_SAVE     0x00B00000
#define PLINC_TYPE_FONTID   0x00C00000

#define PLINC_IS_NULL(x)    (PLINC_TYPE(x) == PLINC_TYPE_NULL)
#define PLINC_IS_MARK(x)    (PLINC_TYPE(x) == PLINC_TYPE_MARK)

/* forward */
struct _PlincInterp;
 
typedef struct _PlincVal PlincVal;
struct _PlincVal {
    PlincUInt               Flags;
    union {
        void               *Ptr;
        PlincInt            Int;
        float               Real;
        struct _PlincOp    *Op;
        void               *(*Func)(struct _PlincInterp *);
    } Val;
};


#endif /* PLINC_TYPES_H */

#ifndef PLINC_TYPES_H
#define PLINC_TYPES_H

#include <sys/types.h>
#include <limits.h>


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#define PLINC_ALIGN         4
#define PLINC_ROUND(x)      (((x) + PLINC_ALIGN - 1) & ~(PLINC_ALIGN - 1))
#define PLINC_LINK(o)       (*(((void **)(o)) - 1))

typedef long                PlincInt;
typedef unsigned long       PlincUInt;
typedef u_int32_t           PlincUInt32;
typedef float               PlincReal;

#define PLINCINT_MAX        0x7fffffff

#define PLINC_ATTR_LIT      0x80000000
#define PLINC_ATTR_NOREAD   0x40000000
#define PLINC_ATTR_NOWRITE  0x20000000
#define PLINC_ATTR_NOEXEC   0x10000000
#define PLINC_ATTR_DOEXEC   0x08000000
#define PLINC_ACCESS_MASK   (PLINC_ATTR_NOREAD | PLINC_ATTR_NOWRITE \
                             | PLINC_ATTR_NOEXEC)
#define PLINC_TYPE_MASK     0x07F00000
#define PLINC_SIZE_MASK     0x000FFFFF

#define PLINC_MAXNAMELEN    255
#define PLINC_MAXLEN        (PLINC_SIZE_MASK - 1)

#define PLINC_LIT(x)        ((x).Flags & PLINC_ATTR_LIT)
#define PLINC_EXEC(x)       (!((x).Flags & PLINC_ATTR_LIT))
#define PLINC_DOEXEC(x)     ((x).Flags & PLINC_ATTR_DOEXEC)
#define PLINC_CAN_READ(x)   (!((x).Flags & PLINC_ATTR_NOREAD))
#define PLINC_CAN_WRITE(x)  (!((x).Flags & PLINC_ATTR_NOWRITE))
#define PLINC_CAN_EXEC(x)   (!((x).Flags & PLINC_ATTR_NOEXEC))

#define PLINC_TYPE(x)       ((x).Flags & PLINC_TYPE_MASK)
#define PLINC_SIZE(x)       ((x).Flags & PLINC_SIZE_MASK)
#define PLINC_SET_SIZE(x,s) ((x).Flags = ((x).Flags & ~PLINC_SIZE_MASK) | (s))

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
#define PLINC_TYPE_SAVE     0x00B00000
#define PLINC_TYPE_FONTID   0x00C00000

#define PLINC_IS_INT(x)     (PLINC_TYPE(x) == PLINC_TYPE_INT)
#define PLINC_IS_REAL(x)    (PLINC_TYPE(x) == PLINC_TYPE_REAL)
#define PLINC_IS_STRING(x)  (PLINC_TYPE(x) == PLINC_TYPE_STRING)
#define PLINC_IS_FILE(x)    (PLINC_TYPE(x) == PLINC_TYPE_FILE)
#define PLINC_IS_NULL(x)    (PLINC_TYPE(x) == PLINC_TYPE_NULL)
#define PLINC_IS_MARK(x)    (PLINC_TYPE(x) == PLINC_TYPE_MARK)

#define PLINC_IS_NUM(x)     (PLINC_IS_INT(x) || PLINC_IS_REAL(x))
#define PLINC_NUM_VAL(x)    (PLINC_IS_INT(x) ? ((x).Val.Int) : ((x).Val.Real))

/* forward */
struct _PlincInterp;
 
typedef struct _PlincVal PlincVal;
struct _PlincVal {
    PlincUInt32             Flags;
    union {
        void               *Ptr;
        PlincInt            Int;
        float               Real;
        struct _PlincOp    *Op;
    } Val;
};


/* buffer size needed for formatting a plinc number */
#define PLINC_FMT_BUFLEN      (sizeof(PlincInt) * CHAR_BIT + 2)


#endif /* PLINC_TYPES_H */

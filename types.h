/* $Endicor: types.h,v 1.1 1999/01/12 22:27:34 tsarna Exp tsarna $ */

#include <sys/types.h>


#define PLINC_ALIGN         4
#define PLINC_ROUND(x)      (((x) + PLINC_ALIGN - 1) & ~(PLINC_ALIGN - 1))
#define PLINC_LINK(o)       (*(((void **)(o)) - 1))

typedef int32_t     PlincInt;
typedef u_int32_t   PlincUInt;

#define PLINC_ATTR_LIT      0x80000000
#define PLINC_ATTR_WRITE    0x40000000
#define PLINC_ATTR_EXEC     0x20000000
#define PLINC_TYPE_MASK     0x0000FFFF

#define PLINC_LIT(x)        ((x) & PLINC_ATTR_LIT)
#define PLINC_CAN_WRITE(x)  ((x) & PLINC_ATTR_WRITE)
#define PLINC_CAN_EXEC(x)   ((x) & PLINC_ATTR_EXEC)

#define PLINC_TYPE(x)       ((x) & PLINC_TYPE_MASK)

#define PLINC_TYPE_INT      0
#define PLINC_TYPE_REAL     1
#define PLINC_TYPE_BOOL     2
#define PLINC_TYPE_ARRAY    3
#define PLINC_TYPE_STRING   4
#define PLINC_TYPE_NAME     5
#define PLINC_TYPE_DICT     6
#define PLINC_TYPE_OP       7
#define PLINC_TYPE_FILE     8
#define PLINC_TYPE_MARK     9
#define PLINC_TYPE_NULL     10
#define PLINC_TYPE_SAVE     11
#define PLINC_TYPE_FONTID   12

typedef struct _PlincVal PlincVal;
struct _PlincVal {
    PlincUInt   Flags;
    void       *Ptr;
    union       {
        PlincInt    Int;
        float       Real;
    } Val;
};


#define PLINC_VAL_SIZE  PLINC_ROUND(sizeof(PlincVal))

#include <plinc/types.h>
#include <plinc/file.h>

/*
 *  default implementation of bytesavailable for a file
 */
PlincInt
plinc_io_bytesavailable(PlincFile *f)
{
    return -1;
}

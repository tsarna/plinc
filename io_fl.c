#include <plinc/types.h>
#include <plinc/file.h>

/*
 *  default implementation of readtoeof/flushout/purge for a file
 */
int
plinc_io_flushops(PlincFile *f)
{
    return 0;
}

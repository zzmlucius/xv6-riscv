#include "types.h"

struct sysinfo {
    uint64 freemem; // number of bytes of free memory
    uint64 nproc;   // number of processes whose state is not UNUSED
};
#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"


int main(int argc, char *argv[])
{
    struct sysinfo si;

    if(info(&si) == 0) {
        printf("Freemem : %lu  Proc not UNUSED : %lu\n", si.freemem, si.nproc);
        exit(0);
    }
}

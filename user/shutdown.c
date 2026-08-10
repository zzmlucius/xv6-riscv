#include "kernel/types.h"
#include "user/user.h"
int main(int argc, char *argv[])
{
    printf("syncing filesystem...\n");
    if(sync() < 0) {
        fprintf(2, "shutdown : sync failed.");
        exit(1);
    }
    
    printf("xv6 poweroff...");
    shutdown();

    fprintf(2, "shutdown : shutdown failed\n");
    exit(1);
}
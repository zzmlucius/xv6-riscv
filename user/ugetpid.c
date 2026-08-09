#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/memlayout.h"
#include "user/user.h"

int main(int argc, char* argv[])
{
    if(argc > 1) {
        fprintf(2, "too much arguments.");
    }

    struct usyscall *p = (struct usyscall *)USYSCALL;
    printf("ugetpid : %d\n", p -> pid);

    exit(0);
}
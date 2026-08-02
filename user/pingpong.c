#include "kernel/types.h"
#include "user/user.h"

int main(int argc, int *argv[])
{
    int fd1[2];
    int fd2[2];
    char str;
    pipe(fd1);
    pipe(fd2);

    int rc = fork();
    if(rc < 0) {
        fprintf(2, "fork fail.");
        exit(1);
    }
    // child
    if(rc == 0) {
        close(fd1[0]);
        close(fd2[1]);
        if(read(fd2[0], &str, 1) == 1) { //success read 1 byte
            printf("<%d> : receive ping.\n", getpid());
            write(fd1[1], "h", 1);      //success write 1 byte
            exit(0);
        }
    }

    //parent
    
    close(fd1[1]);
    close(fd2[0]);
    write(fd2[1], "h", 1);      //success write 1 byte
    if(read(fd1[0], &str, 1) == 1) {
        printf("<%d> : receive pong.\n", getpid());
    }
    wait(0);
    exit(0);
}
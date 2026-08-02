#include "kernel/types.h"
#include "user/user.h"


int main(int argc, char *argv[])
{
    if(argc <= 1) {
        fprintf(2, "Too few arguments, type 'sleep n(n seconds)'.\n");
        exit(1);
    }

    int seconds = atoi(argv[1]) * 10; //change char to int\

    if(seconds < 0) {
        fprintf(2, "invalid time.\n");
        exit(1);
    }

    if(pause(seconds) == 0) exit(0);
    
    else {
        fprintf(2, "fail to sleep.\n");
        exit(1);
    }
}
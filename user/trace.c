#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if(argc < 3) { // at leatst three args, 1.trace 2.mask 3.call user func
        fprintf(2, "trace : too few arguments.");
        exit(1);
    }

    uint64 mask = atoi(argv[1]);
    int n = argc - 2;
    char *nargv[n]; 
    //
    for(int i = 0;i < n;i++) 
        nargv[i] = argv[i + 2];
    
    nargv[n] = 0; // Attention : argv[]参数必须以 0 结尾

    if(mask < 0 || mask > 2147483647) {
        fprintf(2, "trace : mask out of range.");
    }

    trace(mask);

    exec(nargv[0], nargv); // Attention : exec()后的代码跟 exit(1)

    fprintf(2, "trace: exec %s failed\n", nargv[0]);

    exit(1);
}
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) //char *指向字符串或字符数组后者要以'\0'结尾，char *argv[]要以NULL结尾
{
    char c;
    int len = 0;
    char line[512];
    char *args[MAXARG]; //MAXARG = 32
    if(argc < 2) {
        fprintf(2, "xargs : too few arguments.");
        exit(1);
    }

    while(read(0, &c, 1) == 1) { //读一行执行一行
        if(c == '\n') {     //按空格分割，创建新args[]，exec
            for(int i = 1;i < argc;i++) {
                args[i - 1] = argv[i];
            }
            line[len] = '\0';
            int n = argc - 1;
            int start = 0;

            for(int i = 0;i <= len;i++) {
                if(line[i] == ' ' || line[i] == '\0') { //' '是空格，'\0'是字符串结尾标志，'\n'是换行标志
                    line[i] = '\0';  //方便下一行的直接读取。
                    args[n++] = &line[start];
                    start = i + 1;
                }
            }
            args[n] = 0;
            //已经完成尾接。

            if(fork() == 0) {
                exec(args[0], args);
                exit(1); //执行到这一行说明出问题
            }
            
            wait(0);
            len = 0;
        }

        else line[len++] = c;
    }
} // char line[512]，通过添加'\0'可以当成 变长单词存储器。
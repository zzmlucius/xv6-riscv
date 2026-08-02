#include "kernel/types.h"
#include "user/user.h"
int main()
{
    int fd[2];
    pipe(fd);

    int x;        
    int *buf = &x; //这样才算初始化成功

    int array[34];
    for(int i = 0;i < 34;i++) {
        array[i] = i + 2;
    }

    int pid = getpid();

    while(fork() == 0) //注意分析这种写法下的代码行为
    {
        close(fd[1]);  //子进程不用向左管道写
        if(read(fd[0], buf, sizeof(int)) != 0) {
            int prime = *buf;
            printf("prime : %d\n", prime);

            int f = fd[0]; //保存上一个pipe，创建新pipe
            pipe(fd);

            while(read(f, buf, sizeof(int)))//从上一个pipe读 向下一个pipe写
                if(*buf % prime) 
                    write(fd[1], buf, 4);
            
            close(f);
            close(fd[1]); //右管道已写完
        }
        else {
            close(fd[0]);
            break;
        }
    }
    //  !!!:假设这里P运行到while(fork() == 0) 产生C1，P中fork() != 0，故离开while循环。
    //  C1进入while循环执行if
    //  if执行完毕，回到开头创建C2
    //!:此时C1fork() != 0，退出循环
  
    if(getpid() == pid) { //是父进程
        close(fd[0]);
        write(fd[1], array, 136);
        close(fd[1]);
        wait(0);
        exit(0);
    }

    else {
        close(fd[0]);
        wait(0);         //等待子进程创建的子进程
        exit(0);         //结束while出来的子进程
    }
    
} 
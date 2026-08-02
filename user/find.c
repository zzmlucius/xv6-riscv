#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"


char *fmtname(char *path) //返回path最后一个文件名
{
  static char buf[DIRSIZ + 1];
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  buf[strlen(p)] = '\0';
  return buf;
}

void find(char *path, char *obj) //always receive the full path
{
    char buf[512], *p;            // using to connect the path.
    int fd;
    struct dirent de;   // using for reading the directory.
    struct stat st;     // using to load the file.

    if((fd = open(path, O_RDONLY)) < 0) { // fail to open file
        fprintf(2, "cannot open %s\n.", path);
        return;
    }

    if(fstat(fd, &st) < 0) { //stat -> fstat -> filestat() //将数据写入st
        fprintf(2, "cannot stat %s\n.", path);
        close(fd);
        return;
    }

    switch(st.type) {
        case T_DEVICE:
        case T_FILE:
            if(strcmp(fmtname(path), obj) == 0) 
                printf("%s\n", path);
            break;
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf("ls: path too long\n");
                break;
            }
            
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)) {
                if(de.inum == 0) //directory had been deleted
                    continue;
                if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;
                memmove(p, de.name, DIRSIZ); //拼接成完整路径
                p[DIRSIZ] = 0;
                
                find(buf, obj); // 传入全路径
            }
            break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if(argc != 3) { 
        fprintf(2, "only receive 3 arguments.");
        exit(1);
    }
    
    find(argv[1], argv[2]);
    exit(0);
}
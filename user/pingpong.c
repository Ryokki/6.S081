#include "../kernel/types.h"
#include "user.h"
/*pipe(arr):arr[0] read , arr[1] write*/
int 
main(int argc, char** argv ){
    int pid;
    int parent_fd[2];
    int child_fd[2];
    char buf[20];
    //为父子进程建立管道
    pipe(child_fd); 
    pipe(parent_fd);

    int child_write  = child_fd[1];
    int child_read   = parent_fd[0];
    int parent_write = parent_fd[1];
    int parent_read  = child_fd[0];

    // Child Progress
    if((pid = fork()) == 0){
        close(parent_write);  
        read(child_read,buf, 4);
        printf("%d: received %s\n",getpid(), buf);
        close(child_read); 
        write(child_write, "pong", sizeof(buf));
        exit(0);
    }
    // Parent Progress
    else{
        close(child_read);
        write(parent_write, "ping",4);
        close(child_write);
        read(parent_read, buf, sizeof(buf));
        printf("%d: received %s\n", getpid(), buf);
        exit(0);
    }

}

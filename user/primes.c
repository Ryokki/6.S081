#include "../kernel/types.h"
#include "user.h"

void f(int pp) {
    int prime;
    if (read(pp, (char *)&prime, 4) != 0){  //读出传进来的第一个数
        printf("prime %d\n", prime);    //直接打印出来.
        int p[2]; 
        pipe(p);    ////p[0]:R , P[1]:W
        // child
        if (fork() == 0) {
            close(p[1]); 
            f(p[0]);
        } 
        // parent
        else {
            int t;
            while (read(pp, (char *)&t, 4) != 0) {  
                if (t % prime != 0) write(p[1], (char *)&t, 4);
            }
            close(p[1]); close(p[0]); close(pp); 
            wait(0);
        }
    } else close(pp);   //递归终止条件——父进程没有传递.
}
int main() {
    int i, p[2]; 
    pipe(p);    //p[0]:R , P[1]:W
    // child
    if (fork() == 0) {
        close(p[1]); f(p[0]);   //R
    }
    // parent 
    else {
        for (i = 2; i < 36; i++) 
            write(p[1], (char *)&i, 4);
        close(p[1]); close(p[0]); 
        wait(0);
    }
    exit(0);
}  
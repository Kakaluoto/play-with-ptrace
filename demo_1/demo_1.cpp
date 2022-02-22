#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>

using namespace std;

int main() {
    pid_t child;
    long orig_rax, rax;
    long params[3] = {0};
    int status;
    int insyscall = 0;
    int cnt = 0;
    child = fork();
    if (child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    } else {
        while (true) {
            wait(&status);
            cout << "counting:"<<cnt++ << endl;
            if (WIFEXITED(status)){
                cout<<"WIFEXITED true"<<endl;
                break;
            } else{
                cout<<"WIFEXITED false"<<endl;
            }
            orig_rax = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
            //printf("the child made a system call %ld\n",orig_rax);
            if (orig_rax == SYS_write) {
                if (insyscall == 0) {
                    insyscall = 1;
                    params[0] = ptrace(PTRACE_PEEKUSER, child, 8 * RDI, NULL);
                    params[1] = ptrace(PTRACE_PEEKUSER, child, 8 * RSI, NULL);
                    params[2] = ptrace(PTRACE_PEEKUSER, child, 8 * RDX, NULL);
                    printf("write called with %ld, %ld, %ld\n", params[0], params[1], params[2]);
                } else {
                    rax = ptrace(PTRACE_PEEKUSER, child, 8 * RAX, NULL);
                    printf("write returned with %ld\n", rax);
                    insyscall = 0;
                }
            }
            ptrace(PTRACE_SYSCALL, child, NULL, NULL);
        }
    }
    return 0;

}
/***
 输出：
 write called with 1, 25226320, 65
 ptrace_1.c ptrace_2.c ptrace_3.C ptrace_4.C ptrace_5.c test.c
 write returned with 65
 ***/
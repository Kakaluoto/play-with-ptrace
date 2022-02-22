#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <cstdio>
#include <unistd.h>
#include <iostream>

#define LONG_SIZE 8

using namespace std;

int main() {
    pid_t child;
    child = fork();
    if (child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);//这里的test是一个输出hello world的小程序
        execl("./hello", "hello", NULL);
    } else {
        int status = 0;
        struct user_regs_struct regs;
        int start = 0;
        long ins;
        int cnt = 0;
        while (true) {
            wait(&status);
            cout<<"counting:"<<cnt++;
            if (WIFSTOPPED(status))
                cout<<" SignalNumber:"<<WSTOPSIG(status)<<endl;
            if (WIFEXITED(status))
                break;
            ptrace(PTRACE_GETREGS, child, NULL, &regs);
            if (start == 1) {
                ins = ptrace(PTRACE_PEEKTEXT, child, regs.rip,NULL);
                printf("RIP:%lx Instruction executed:%lx\n", regs.rip, ins);
            }
            if (regs.orig_rax == SYS_write) {
                start = 1;
                cout<<"Into SYS_write\n";
                ptrace(PTRACE_SINGLESTEP, child, NULL, NULL);
            } else {
                cout<<"PTRACE_SYSCALL\n";
                ptrace(PTRACE_SYSCALL, child, NULL, NULL);
            }
        }
    }
}
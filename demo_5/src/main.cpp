#include <sys/types.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    pid_t traced_process;
    struct user_regs_struct regs;
    long ins;
    if (argc != 2) {
        puts("no pid input");
        exit(1);
    }
    traced_process = atoi(argv[1]);
    printf("try to trace pid :%u\n", traced_process);
    if (ptrace(PTRACE_ATTACH, traced_process, nullptr, nullptr) == -1) {
        perror("trace error:");
    }
    wait(nullptr);
    if (ptrace(PTRACE_GETREGS, traced_process, nullptr, &regs) == -1) {
        perror("trace error:");
    }
    ins = ptrace(PTRACE_PEEKTEXT, traced_process, regs.rip, nullptr);
    if (ins == -1) {
        perror("trace error:");
    }
//    while (true){
//        if (ptrace(PTRACE_GETREGS, traced_process, nullptr, &regs) != -1) {
//            printf("RIP:%llx \n", regs.rip);
//        }
//    }
    printf("RIP:%llx Instruction executed: %lx\n", regs.rip, ins);
    if (ptrace(PTRACE_DETACH, traced_process, nullptr, nullptr) == -1) {
        perror("trace error:");
    }

    return 0;
}
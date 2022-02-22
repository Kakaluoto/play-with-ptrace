#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#define LONG_SIZE 8

using namespace std;

void getdata(pid_t child, long addr, char* str, int len) {
    char* laddr = str;
    int i = 0, j = len / LONG_SIZE;
    union u {
        long val;
        char chars[LONG_SIZE];
    } word{};
    while (i < j) {
        word.val = ptrace(PTRACE_PEEKDATA, child, addr + i * LONG_SIZE, nullptr);
        if (word.val == -1)
            perror("trace error");
        memcpy(laddr, word.chars, LONG_SIZE);
        ++i;
        laddr += LONG_SIZE;
    }
    j = len % LONG_SIZE;
    if (j != 0) {
        word.val = ptrace(PTRACE_PEEKDATA, child, addr + i * LONG_SIZE, nullptr);
        if (word.val == -1)
            perror("trace error");
    }
    str[len] = '\0';
}


void putdata(pid_t child, long addr, char* str, int len) {
    char* laddr = str;
    int i = 0, j = len / LONG_SIZE;
    union u {
        long val;
        char chars[LONG_SIZE];
    } word{};
    while (i < j) {
        memcpy(word.chars, laddr, LONG_SIZE);
        if (ptrace(PTRACE_POKEDATA, child, addr + i * LONG_SIZE, word.val) == -1)
            perror("trace error");
        ++i;
        laddr += LONG_SIZE;
    }
    j = len % LONG_SIZE;
    if (j != 0) {
        word.val = 0;
        memcpy(word.chars, laddr, j);
        if (ptrace(PTRACE_POKEDATA, child, addr + i * LONG_SIZE, word.val) == -1)
            perror("trace error");
    }
}


void printBytes(const char* tip, char* codes, int len) {
    int i;
    printf("%s :", tip);
    for (i = 0; i < len; ++i) {
        printf("%02x ", (unsigned char) codes[i]);
    }
    puts("");
}

#define CODE_SIZE 8

int main(int argc, char* argv[]) {
    if (argc != 2) {
        puts("no pid input");
        exit(1);
    }
    pid_t traced_process;
    struct user_regs_struct regs{};
    struct user_regs_struct temp_regs{};
    char code[LONG_SIZE] = {static_cast<char>(0xcc)};
    char backup[LONG_SIZE];
    traced_process = strtol(argv[1], nullptr, 10);
    printf("try to attach pid:%u\n", traced_process);
    if (ptrace(PTRACE_ATTACH, traced_process, nullptr, nullptr) == -1) {
        perror("trace attach error");
    }
    wait(nullptr);
    if (ptrace(PTRACE_GETREGS, traced_process, nullptr, &regs) == -1) {
        perror("trace get regs error");
    }
    //copy instructions into backup variable
    getdata(traced_process, regs.rip, backup, CODE_SIZE*4);
    printBytes("get tracee instruction", backup, LONG_SIZE);
    puts("try to set breakpoint");
    printBytes("set breakpoint instruction", code, LONG_SIZE);
    putdata(traced_process, regs.rip, code, CODE_SIZE);
    if (ptrace(PTRACE_CONT, traced_process, nullptr, nullptr) == -1) {
        perror("trace continue error");
    }
    wait(nullptr);
//    puts("the process stopped Press <Enter> to continue");
//    getchar();
    printBytes("place breakpoint instruction with tracee instruction", backup, LONG_SIZE);
    putdata(traced_process, regs.rip, backup, CODE_SIZE);
    ptrace(PTRACE_SETREGS, traced_process, nullptr, &regs);
    ptrace(PTRACE_DETACH, traced_process, nullptr, nullptr);
//    int status;
//    while (true) {
//        ptrace(PTRACE_SINGLESTEP, traced_process, nullptr, nullptr);
//        wait(&status);
//        if (WIFSTOPPED(status)) {
//            printf("%d\n",status);
//        }
//        printf("hey!\n");
//    }
    return 0;
}
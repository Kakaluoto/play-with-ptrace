#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LONG_SIZE 8

//获取参数
char* getdata(pid_t child, unsigned long addr, unsigned long len) {
    char* str = (char*) malloc(len + 1);
    memset(str, 0, len + 1);
    union u {
        long int val;
        char chars[LONG_SIZE];
    } word;
    int i, j;
    for (i = 0, j = len / LONG_SIZE; i < j; ++i) {
        word.val = ptrace(PTRACE_PEEKDATA, child, addr + i * LONG_SIZE, NULL);
        if (word.val == -1)
            perror("trace get data error");
        memcpy(str + i * LONG_SIZE, word.chars, LONG_SIZE);
    }
    j = len % LONG_SIZE;
    if (j != 0) {
        word.val = ptrace(PTRACE_PEEKDATA, child, addr + i * LONG_SIZE, NULL);
        if (word.val == -1)
            perror("trace get data error");
        memcpy(str + i * LONG_SIZE, word.chars, j);
    }
    return str;
}

//提交参数
void putdata(pid_t child, unsigned long addr, unsigned long len, char* newstr) {
    union u {
        long val;
        char chars[LONG_SIZE];
    } word;
    int i, j;
    for (i = 0, j = len / LONG_SIZE; i < j; ++i) {
        memcpy(word.chars, newstr + i * LONG_SIZE, LONG_SIZE);
        if (ptrace(PTRACE_POKEDATA, child, addr + i * LONG_SIZE, word.val) == -1)
            perror("trace error");

    }
    j = len % LONG_SIZE;
    if (j != 0) {
        memcpy(word.chars, newstr + i * LONG_SIZE, j);
        ptrace(PTRACE_POKEDATA, child, addr + i * LONG_SIZE, word.val);
    }
}

//修改参数
void reserve(char* str, unsigned int len) {
    int i, j;
    char tmp;
    for (i = 0, j = len - 2; i <= j; ++i, --j) {
        tmp = str[i];
        str[i] = str[j];
        str[j] = tmp;
    }
}

int main() {
    pid_t child;
    child = fork();
    if (child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    } else {
        struct user_regs_struct regs;
        int status = 0;
        int toggle = 0;
        while (true) {
            wait(&status);
            if (WIFEXITED(status))
                break;
            memset(&regs, 0, sizeof(struct user_regs_struct));
            if (ptrace(PTRACE_GETREGS, child, NULL, &regs) == -1) {
                perror("trace error");
            }

            if (regs.orig_rax == SYS_write) {
                if (toggle == 0) {
                    toggle = 1;
                    //in x86_64 system call ,pass params with %rdi, %rsi, %rdx, %rcx, %r8, %r9
                    //no system call has over six params
                    printf("make write call params %llu, %llu, %llu\n", regs.rdi, regs.rsi, regs.rdx);
                    char* str = getdata(child, regs.rsi, regs.rdx);
                    printf("old str,len %lu:\n%s", strlen(str), str);
                    reserve(str, regs.rdx);
                    printf("hook str,len %lu:\n%s", strlen(str), str);
                    putdata(child, regs.rsi, regs.rdx, str);
                    free(str);
                } else {
                    toggle = 0;
                }
            }
            ptrace(PTRACE_SYSCALL, child, NULL, NULL);
        }
    }
    return 0;
}
/***
输出：
make write call params 1, 9493584, 66
old str,len 66:
ptrace        ptrace2    ptrace3     ptrace4    ptrace5     test    test.s
hook str,len 66:
s.tset    tset     5ecartp    4ecartp     3ecartp    2ecartp        ecartp
s.tset    tset     5ecartp    4ecartp     3ecartp    2ecartp        ecartp
make write call params 1, 9493584, 65
old str,len 65:
ptrace_1.c  ptrace_2.c    ptrace_3.C  ptrace_4.C    ptrace_5.c  test.c
hook str,len 65:
c.tset  c.5_ecartp    C.4_ecartp  C.3_ecartp    c.2_ecartp  c.1_ecartp
c.tset  c.5_ecartp    C.4_ecartp  C.3_ecartp    c.2_ecartp  c.1_ecartp
***/

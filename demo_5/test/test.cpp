#include<cstdio>
#include <unistd.h>
int main() {
    int i;
    for (i = 0; i >= 0; ++i) {
        printf("My counter: %d \n", i);
        sleep(1);
    }
    return 0;
}//
// Created by hy on 2022/2/12.
//


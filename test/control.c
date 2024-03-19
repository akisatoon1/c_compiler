#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

// 何番目のテストかを表す数。最初は1
int num;
int error_num;
int ASSERT(int expected, int actual)
{
    if (expected != actual)
    {
        error_num++;
        printf("\x1b[31m");
        printf("\ntest%d error!\n", num);
        printf("\x1b[0m");
        printf("expected is %d, but actual is %d\n", expected, actual);
    }
    num++;
    return 0;
}

int main()
{
    printf("\n\n\x1b[34mcontrol.c:\x1b[0m \n");
    num = 1;
    error_num = 0;

    ASSERT(3, ({ int x; if (0) x=2; else x=3; x; })); // 1
    ASSERT(3, ({ int x; if (1-1) x=2; else x=3; x; })); // 2
    ASSERT(2, ({ int x; if (1) x=2; else x=3; x; })); // 3
    ASSERT(2, ({ int x; if (2-1) x=2; else x=3; x; })); // 4

    ASSERT(55, ({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; })); // 5

    ASSERT(10, ({ int i=0; while(i<10) i=i+1; i; })); // 6

    ASSERT(10, ({ int i=0; while(i<10) i=i+1; i; })); // 7
    ASSERT(55, ({ int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} j; })); // 8

    ASSERT(3, ({ int i=0; for(;i<10;i++) { if (i == 3) break; } i; })); // 9
    ASSERT(4, ({ int i=0; while (1) { if (i++ == 3) break; } i; })); // 10
    ASSERT(3, ({ int i=0; for(;i<10;i++) { for (;;) break; if (i == 3) break; } i; })); // 11
    ASSERT(4, ({ int i=0; while (1) { while(1) break; if (i++ == 3) break; } i; })); // 12

    if (error_num == 0)
        printf("OK\n");
    else
        printf("\nthe number of error is %d\n", error_num);
    return 0;
}

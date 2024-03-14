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
    printf("\n\n\x1b[34mpointer.c:\x1b[0m \n");
    num = 1;
    error_num = 0;

    ASSERT(3, ({ int x=3; *&x; })); // 1
    ASSERT(3, ({ int x=3; int *y=&x; int **z=&y; **z; })); // 2
    ASSERT(5, ({ int x=3; int y=5; *(&x-1); })); // 3
    ASSERT(3, ({ int x=3; int y=5; *(&y+1); })); // 4
    ASSERT(5, ({ int x=3; int y=5; *(&x-(-1)); })); // 5
    ASSERT(5, ({ int x=3; int *y=&x; *y=5; x; })); // 6
    ASSERT(7, ({ int x=3; int y=5; *(&x-1)=7; y; })); // 7
    ASSERT(7, ({ int x=3; int y=5; *(&y+2-1)=7; x; })); // 8
    ASSERT(5, ({ int x=3; (&x+2)-&x+3; })); // 9
    ASSERT(8, ({ int x; int y; x=3; y=5; x+y; })); // 10
    ASSERT(8, ({ int x=3; int y=5; x+y; })); // 11

    ASSERT(3, ({ int x[2]; int *y=&x; *y=3; *x; })); // 12

    ASSERT(3, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; })); // 13
    ASSERT(4, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); })); // 14
    ASSERT(5, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); })); // 15

    ASSERT(0, ({ int x[2][3]; int *y=x; *y=0; **x; })); // 16
    ASSERT(1, ({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); })); // 17
    ASSERT(2, ({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); })); // 18
    ASSERT(3, ({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); })); // 19
    ASSERT(4, ({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); })); // 20
    ASSERT(5, ({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); })); // 21

    ASSERT(3, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; })); // 22
    ASSERT(4, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); })); // 23
    ASSERT(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); })); // 24
    ASSERT(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); })); // 25
    ASSERT(5, ({ int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2); })); // 26

    ASSERT(0, ({ int x[2][3]; int *y=x; y[0]=0; x[0][0]; })); // 27
    ASSERT(1, ({ int x[2][3]; int *y=x; y[1]=1; x[0][1]; })); // 28
    ASSERT(2, ({ int x[2][3]; int *y=x; y[2]=2; x[0][2]; })); // 29
    ASSERT(3, ({ int x[2][3]; int *y=x; y[3]=3; x[1][0]; })); // 30
    ASSERT(4, ({ int x[2][3]; int *y=x; y[4]=4; x[1][1]; })); // 31
    ASSERT(5, ({ int x[2][3]; int *y=x; y[5]=5; x[1][2]; })); // 32

    if (error_num == 0)
        printf("OK\n");
    else
        printf("\nthe number of error is %d\n", error_num);
    return 0;
}

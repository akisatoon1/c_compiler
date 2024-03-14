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

int g1;
int g2[4];

int main()
{
    printf("\n\x1b[34mvariable.c:\x1b[0m \n");
    num = 1;
    error_num = 0;

    ASSERT(3, ({ int a; a=3; a; })); // 1
    ASSERT(3, ({ int a=3; a; })); // 2
    ASSERT(8, ({ int a=3; int z=5; a+z; })); // 3

    ASSERT(3, ({ int a=3; a; })); // 4
    ASSERT(8, ({ int a=3; int z=5; a+z; })); // 5
    ASSERT(6, ({ int a; int b; a=b=3; a+b; })); // 6
    ASSERT(3, ({ int foo=3; foo; })); // 7
    ASSERT(8, ({ int foo123=3; int bar=5; foo123+bar; })); // 8

    ASSERT(4, ({ int x; sizeof(x); }));  // 9
    ASSERT(4, ({ int x; sizeof x; }));  // 10
    ASSERT(8, ({ int *x; sizeof(x); }));  // 11
    ASSERT(16, ({ int x[4]; sizeof(x); })); // 12
    ASSERT(48, ({ int x[3][4]; sizeof(x); })); // 13
    ASSERT(16, ({ int x[3][4]; sizeof(*x); })); // 14
    ASSERT(4, ({ int x[3][4]; sizeof(**x); }));  // 15
    ASSERT(5, ({ int x[3][4]; sizeof(**x) + 1; }));  // 16
    ASSERT(5, ({ int x[3][4]; sizeof **x + 1; }));  // 17
    ASSERT(4, ({ int x[3][4]; sizeof(**x + 1); }));  // 18
    ASSERT(4, ({ int x=1; sizeof(x=2); }));  // 19
    ASSERT(1, ({ int x=1; sizeof(x=2); x; }));  // 20

    ASSERT(0, g1);    // 21
    ASSERT(3, ({ g1=3; g1; })); // 22
    ASSERT(0, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[0]; })); // 23
    ASSERT(1, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[1]; })); // 24
    ASSERT(2, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[2]; })); // 25
    ASSERT(3, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[3]; })); // 26

    ASSERT(4, sizeof(g1));  // 27
    ASSERT(16, sizeof(g2)); // 28

    ASSERT(1, ({ char x=1; x; })); // 29
    ASSERT(1, ({ char x=1; char y=2; x; })); // 30
    ASSERT(2, ({ char x=1; char y=2; y; })); // 31

    ASSERT(1, ({ char x; sizeof(x); }));  // 32
    ASSERT(10, ({ char x[10]; sizeof(x); })); // 33

    ASSERT(2, ({ int x=2; { int x=3; } x; })); // 34
    ASSERT(2, ({ int x=2; { int x=3; } int y=4; x; })); // 35
    ASSERT(3, ({ int x=2; { x=3; } x; })); // 36

    ASSERT(1, ({ int x; int y; char z; char *a=&y; char *b=&z; a-b; })); // 37
    ASSERT(4, ({ int x; char y; int z; char *a=&y; char *b=&z; a-b; })); // 38

    if (error_num == 0)
        printf("OK\n");
    else
        printf("\nthe number of error is %d\n", error_num);
    return 0;
}

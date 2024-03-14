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

int ret3()
{
    return 3;
    return 5;
}

int add2(int x, int y)
{
    return x + y;
}

int sub2(int x, int y)
{
    return x - y;
}

int add6(int a, int b, int c, int d, int e, int f)
{
    return a + b + c + d + e + f;
}

int addx(int *x, int y)
{
    return *x + y;
}

int sub_char(char a, char b, char c)
{
    return a - b - c;
}

int fib(int x)
{
    if (x <= 1)
        return 1;
    return fib(x - 1) + fib(x - 2);
}

int main()
{
    printf("\n\n\x1b[34mfunction.c:\x1b[0m \n");
    num = 1;
    error_num = 0;

    ASSERT(3, ret3());                                                                    // 1
    ASSERT(8, add2(3, 5));                                                                // 2
    ASSERT(2, sub2(5, 3));                                                                // 3
    ASSERT(21, add6(1, 2, 3, 4, 5, 6));                                                   // 4
    ASSERT(66, add6(1, 2, add6(3, 4, 5, 6, 7, 8), 9, 10, 11));                            // 5
    ASSERT(136, add6(1, 2, add6(3, add6(4, 5, 6, 7, 8, 9), 10, 11, 12, 13), 14, 15, 16)); // 6

    ASSERT(7, add2(3, 4)); // 7
    ASSERT(1, sub2(4, 3)); // 8
    ASSERT(55, fib(9));    // 9

    ASSERT(1, ({ sub_char(7, 3, 3); })); // 10

    if (error_num == 0)
        printf("OK\n");
    else
        printf("\nthe number of error is %d\n", error_num);
    return 0;
}

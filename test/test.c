#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

// 何番目のテストかを表す数。最初は1
int num;
int assert(int expected, int actual)
{
    if (expected != actual)
    {
        printf("WRONG!\n");
        printf("expected is %d, but actual is %d\n", expected, actual);
        exit(1);
    }
    else
    {
        printf("test%d passed\n", num);
        num = num + 1;
    }
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

int fib(int x)
{
    if (x <= 1)
        return 1;
    return fib(x - 1) + fib(x - 2);
}

int f1(int x)
{
    int y;
    int *z;
    int **a;
    y = 5;
    z = &y;
    a = &z;
    return x + *z + **a;
}

// 10
int f2()
{
    int x[5];
    int a;
    a = 0;
    x[0] = 0;
    x[1] = 1;
    x[2] = 2;
    x[3] = 3;
    x[4] = 4;
    int i;
    for (i = 0; i < 5; i = i + 1)
    {
        a = a + x[i];
    }
    return a;
}

// 4
int f3()
{
    char x;
    x = 4;
    return x;
}

// 24
int f4()
{
    int x[3];
    *x = 0;
    return sizeof(x) + sizeof(x + 1) + sizeof(*x);
}

// 10
int g;
int f5()
{
    int x;
    x = 5;
    g = 5;
    return x + g;
}

// 5
int f6()
{
    int a;
    a = 0;
    int i;
    i = 0;
    while (i < 5)
    {
        if (i < 3)
            a = a + i;
        else
            a = a + 1;
        i = i + 1;
    }
    return a;
}

int main()
{
    num = 1;
    assert(0, 0);
    assert(42, 42);
    assert(21, 5 + 20 - 4);
    assert(41, 12 + 34 - 5);
    assert(47, 5 + 6 * 7);
    assert(15, 5 * (9 - 6));
    assert(4, (3 + 5) / 2);
    assert(10, -10 + 20);
    assert(10, - -10);
    assert(10, - -+10);

    assert(0, 0 == 1);
    assert(1, 42 == 42);
    assert(1, 0 != 1);
    assert(0, 42 != 42);

    assert(1, 0 < 1);
    assert(0, 1 < 1);
    assert(0, 2 < 1);
    assert(1, 0 <= 1);
    assert(1, 1 <= 1);
    assert(0, 2 <= 1);

    assert(1, 1 > 0);
    assert(0, 1 > 1);
    assert(0, 1 > 2);
    assert(1, 1 >= 0);
    assert(1, 1 >= 1);
    assert(0, 1 >= 2);

    assert(3, ret3());
    assert(8, add2(3, 5));
    assert(2, sub2(5, 3));
    assert(21, add6(1, 2, 3, 4, 5, 6));
    assert(66, add6(1, 2, add6(3, 4, 5, 6, 7, 8), 9, 10, 11));
    assert(136, add6(1, 2, add6(3, add6(4, 5, 6, 7, 8, 9), 10, 11, 12, 13), 14, 15, 16));

    assert(7, add2(3, 4));
    assert(1, sub2(4, 3));
    assert(55, fib(9));

    assert(30, f1(20));
    assert(10, f2());
    assert(4, f3());
    assert(24, f4());
    assert(10, f5());
    assert(5, f6()); // 41番目のテスト

    printf("OK\n");
    return 0;
}
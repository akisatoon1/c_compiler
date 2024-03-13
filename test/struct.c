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
    printf("\n\x1b[34mstruct.c:\x1b[0m \n");
    num = 1;
    error_num = 0;

    ASSERT(1, ({ struct {int a; int b;} x; x.a=1; x.b=2; x.a; })); // 1
    ASSERT(2, ({ struct {int a; int b;} x; x.a=1; x.b=2; x.b; })); // 2
    ASSERT(1, ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.a; })); // 3
    ASSERT(2, ({ struct {char a; int b; char c;} x; x.b=1; x.b=2; x.c=3; x.b; })); // 4
    ASSERT(3, ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.c; })); // 5

    ASSERT(0, ({ struct {char a; char b;} x[3]; char *p=x; p[0]=0; x[0].a; })); // 6
    ASSERT(1, ({ struct {char a; char b;} x[3]; char *p=x; p[1]=1; x[0].b; })); // 7
    ASSERT(2, ({ struct {char a; char b;} x[3]; char *p=x; p[2]=2; x[1].a; })); // 8
    ASSERT(3, ({ struct {char a; char b;} x[3]; char *p=x; p[3]=3; x[1].b; })); // 9

    ASSERT(6, ({ struct {char a[3]; char b[5];} x; char *p=&x; x.a[0]=6; p[0]; })); // 10
    ASSERT(7, ({ struct {char a[3]; char b[5];} x; char *p=&x; x.b[0]=7; p[3]; })); // 11

    ASSERT(6, ({ struct { struct { char b; } a; } x; x.a.b=6; x.a.b; })); // 12

    ASSERT(4, ({ struct {int a;} x; sizeof(x); }));  // 13
    ASSERT(8, ({ struct {int a; int b;} x; sizeof(x); }));  // 14
    ASSERT(8, ({ struct {int a; int b;} x; sizeof(x); }));  // 15
    ASSERT(12, ({ struct {int a[3];} x; sizeof(x); })); // 16
    ASSERT(16, ({ struct {int a;} x[4]; sizeof(x); })); // 17
    ASSERT(24, ({ struct {int a[3];} x[2]; sizeof(x); })); // 18
    ASSERT(2, ({ struct {char a; char b;} x; sizeof(x); }));  // 19
    ASSERT(0, ({ struct {} x; sizeof(x); }));  // 20
    ASSERT(5, ({ struct {char a; int b;} x; sizeof(x); }));  // 21
    ASSERT(5, ({ struct {int a; char b;} x; sizeof(x); }));  // 22

    ASSERT(8, ({ struct t {int a; int b;} x; struct t y; sizeof(y); })); // 23
    ASSERT(8, ({ struct t {int a; int b;}; struct t y; sizeof(y); })); // 24
    ASSERT(2, ({ struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y); })); // 25
    ASSERT(3, ({ struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x; })); // 26

    if (error_num == 0)
        printf("OK\n");
    else
        printf("\nthe number of error is %d\n", error_num);
    return 0;
}

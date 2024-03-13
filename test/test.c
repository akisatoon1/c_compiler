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
int ASSERT(int expected, int actual) // 0
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

int g1;
int g2[4];

int main()
{
    num = 1;
    error_num = 0;
    ASSERT(0, 0);            // 1
    ASSERT(42, 42);          // 2
    ASSERT(21, 5 + 20 - 4);  // 3
    ASSERT(41, 12 + 34 - 5); // 4
    ASSERT(47, 5 + 6 * 7);   // 5
    ASSERT(15, 5 * (9 - 6)); // 6
    ASSERT(4, (3 + 5) / 2);  // 7
    ASSERT(10, -10 + 20);    // 8
    ASSERT(10, - -10);       // 9
    ASSERT(10, - -+10);      // 10

    ASSERT(0, 0 == 1);   // 11
    ASSERT(1, 42 == 42); // 12
    ASSERT(1, 0 != 1);   // 13
    ASSERT(0, 42 != 42); // 14

    ASSERT(1, 0 < 1);  // 15
    ASSERT(0, 1 < 1);  // 16
    ASSERT(0, 2 < 1);  // 17
    ASSERT(1, 0 <= 1); // 18
    ASSERT(1, 1 <= 1); // 19
    ASSERT(0, 2 <= 1); // 20

    ASSERT(1, 1 > 0);  // 21
    ASSERT(0, 1 > 1);  // 22
    ASSERT(0, 1 > 2);  // 23
    ASSERT(1, 1 >= 0); // 24
    ASSERT(1, 1 >= 1); // 25
    ASSERT(0, 1 >= 2); // 26

    ASSERT(3, ({ int x; if (0) x=2; else x=3; x; })); // 27
    ASSERT(3, ({ int x; if (1-1) x=2; else x=3; x; })); // 28
    ASSERT(2, ({ int x; if (1) x=2; else x=3; x; })); // 29
    ASSERT(2, ({ int x; if (2-1) x=2; else x=3; x; })); // 30

    ASSERT(55, ({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; })); // 31

    ASSERT(10, ({ int i=0; while(i<10) i=i+1; i; })); // 32

    ASSERT(10, ({ int i=0; while(i<10) i=i+1; i; })); // 33
    ASSERT(55, ({ int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} j; })); // 34

    ASSERT(3, ret3());                                                                    // 35
    ASSERT(8, add2(3, 5));                                                                // 36
    ASSERT(2, sub2(5, 3));                                                                // 37
    ASSERT(21, add6(1, 2, 3, 4, 5, 6));                                                   // 38
    ASSERT(66, add6(1, 2, add6(3, 4, 5, 6, 7, 8), 9, 10, 11));                            // 39
    ASSERT(136, add6(1, 2, add6(3, add6(4, 5, 6, 7, 8, 9), 10, 11, 12, 13), 14, 15, 16)); // 40

    ASSERT(7, add2(3, 4)); // 41
    ASSERT(1, sub2(4, 3)); // 42
    ASSERT(55, fib(9));    // 43

    ASSERT(1, ({ sub_char(7, 3, 3); })); // 44

    ASSERT(3, ({ int x=3; *&x; })); // 45
    ASSERT(3, ({ int x=3; int *y=&x; int **z=&y; **z; })); // 46
    ASSERT(5, ({ int x=3; int y=5; *(&x-1); })); // 47
    ASSERT(3, ({ int x=3; int y=5; *(&y+1); })); // 48
    ASSERT(5, ({ int x=3; int y=5; *(&x-(-1)); })); // 49
    ASSERT(5, ({ int x=3; int *y=&x; *y=5; x; })); // 50
    ASSERT(7, ({ int x=3; int y=5; *(&x-1)=7; y; })); // 51
    ASSERT(7, ({ int x=3; int y=5; *(&y+2-1)=7; x; })); // 52
    ASSERT(5, ({ int x=3; (&x+2)-&x+3; })); // 53
    ASSERT(8, ({ int x; int y; x=3; y=5; x+y; })); // 54
    ASSERT(8, ({ int x=3; int y=5; x+y; })); // 55

    ASSERT(3, ({ int x[2]; int *y=&x; *y=3; *x; })); // 56

    ASSERT(3, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; })); // 57
    ASSERT(4, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); })); // 58
    ASSERT(5, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); })); // 59

    ASSERT(0, ({ int x[2][3]; int *y=x; *y=0; **x; })); // 60
    ASSERT(1, ({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); })); // 61
    ASSERT(2, ({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); })); // 62
    ASSERT(3, ({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); })); // 63
    ASSERT(4, ({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); })); // 64
    ASSERT(5, ({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); })); // 65

    ASSERT(3, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; })); // 66
    ASSERT(4, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); })); // 67
    ASSERT(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); })); // 68
    ASSERT(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); })); // 69
    ASSERT(5, ({ int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2); })); // 70

    ASSERT(0, ({ int x[2][3]; int *y=x; y[0]=0; x[0][0]; })); // 71
    ASSERT(1, ({ int x[2][3]; int *y=x; y[1]=1; x[0][1]; })); // 72
    ASSERT(2, ({ int x[2][3]; int *y=x; y[2]=2; x[0][2]; })); // 73
    ASSERT(3, ({ int x[2][3]; int *y=x; y[3]=3; x[1][0]; })); // 74
    ASSERT(4, ({ int x[2][3]; int *y=x; y[4]=4; x[1][1]; })); // 75
    ASSERT(5, ({ int x[2][3]; int *y=x; y[5]=5; x[1][2]; })); // 76

    ASSERT(1, ({ struct {int a; int b;} x; x.a=1; x.b=2; x.a; })); // 77
    ASSERT(2, ({ struct {int a; int b;} x; x.a=1; x.b=2; x.b; })); // 78
    ASSERT(1, ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.a; })); // 79
    ASSERT(2, ({ struct {char a; int b; char c;} x; x.b=1; x.b=2; x.c=3; x.b; })); // 80
    ASSERT(3, ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.c; })); // 81

    ASSERT(0, ({ struct {char a; char b;} x[3]; char *p=x; p[0]=0; x[0].a; })); // 82
    ASSERT(1, ({ struct {char a; char b;} x[3]; char *p=x; p[1]=1; x[0].b; })); // 83
    ASSERT(2, ({ struct {char a; char b;} x[3]; char *p=x; p[2]=2; x[1].a; })); // 84
    ASSERT(3, ({ struct {char a; char b;} x[3]; char *p=x; p[3]=3; x[1].b; })); // 85

    ASSERT(6, ({ struct {char a[3]; char b[5];} x; char *p=&x; x.a[0]=6; p[0]; })); // 86
    ASSERT(7, ({ struct {char a[3]; char b[5];} x; char *p=&x; x.b[0]=7; p[3]; })); // 87

    ASSERT(6, ({ struct { struct { char b; } a; } x; x.a.b=6; x.a.b; })); // 88

    ASSERT(4, ({ struct {int a;} x; sizeof(x); }));  // 89
    ASSERT(8, ({ struct {int a; int b;} x; sizeof(x); }));  // 90
    ASSERT(8, ({ struct {int a; int b;} x; sizeof(x); }));  // 91
    ASSERT(12, ({ struct {int a[3];} x; sizeof(x); })); // 92
    ASSERT(16, ({ struct {int a;} x[4]; sizeof(x); })); // 93
    ASSERT(24, ({ struct {int a[3];} x[2]; sizeof(x); })); // 94
    ASSERT(2, ({ struct {char a; char b;} x; sizeof(x); }));  // 95
    ASSERT(0, ({ struct {} x; sizeof(x); }));  // 96
    ASSERT(5, ({ struct {char a; int b;} x; sizeof(x); }));  // 97
    ASSERT(5, ({ struct {int a; char b;} x; sizeof(x); }));  // 98

    ASSERT(8, ({ struct t {int a; int b;} x; struct t y; sizeof(y); })); // 99
    ASSERT(8, ({ struct t {int a; int b;}; struct t y; sizeof(y); })); // 100
    ASSERT(2, ({ struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y); })); // 101
    ASSERT(3, ({ struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x; })); // 102

    ASSERT(3, ({ int a; a=3; a; })); // 103
    ASSERT(3, ({ int a=3; a; })); // 104
    ASSERT(8, ({ int a=3; int z=5; a+z; })); // 105

    ASSERT(3, ({ int a=3; a; })); // 106
    ASSERT(8, ({ int a=3; int z=5; a+z; })); // 107
    ASSERT(6, ({ int a; int b; a=b=3; a+b; })); // 108
    ASSERT(3, ({ int foo=3; foo; })); // 109
    ASSERT(8, ({ int foo123=3; int bar=5; foo123+bar; })); // 110

    ASSERT(4, ({ int x; sizeof(x); }));  // 111
    ASSERT(4, ({ int x; sizeof x; }));  // 112
    ASSERT(8, ({ int *x; sizeof(x); }));  // 113
    ASSERT(16, ({ int x[4]; sizeof(x); })); // 114
    ASSERT(48, ({ int x[3][4]; sizeof(x); })); // 115
    ASSERT(16, ({ int x[3][4]; sizeof(*x); })); // 116
    ASSERT(4, ({ int x[3][4]; sizeof(**x); }));  // 117
    ASSERT(5, ({ int x[3][4]; sizeof(**x) + 1; }));  // 118
    ASSERT(5, ({ int x[3][4]; sizeof **x + 1; }));  // 119
    ASSERT(4, ({ int x[3][4]; sizeof(**x + 1); }));  // 120
    ASSERT(4, ({ int x=1; sizeof(x=2); }));  // 121
    ASSERT(1, ({ int x=1; sizeof(x=2); x; }));  // 122

    ASSERT(0, g1);    // 123//123
    ASSERT(3, ({ g1=3; g1; })); // 124//124
    ASSERT(0, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[0]; })); // 125
    ASSERT(1, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[1]; })); // 126
    ASSERT(2, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[2]; })); // 127
    ASSERT(3, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[3]; })); // 128

    ASSERT(4, sizeof(g1));  // 129
    ASSERT(16, sizeof(g2)); // 130

    ASSERT(1, ({ char x=1; x; })); // 131
    ASSERT(1, ({ char x=1; char y=2; x; })); // 132
    ASSERT(2, ({ char x=1; char y=2; y; })); // 133

    ASSERT(1, ({ char x; sizeof(x); }));  // 134
    ASSERT(10, ({ char x[10]; sizeof(x); })); // 135

    ASSERT(2, ({ int x=2; { int x=3; } x; })); // 136
    ASSERT(2, ({ int x=2; { int x=3; } int y=4; x; })); // 137
    ASSERT(3, ({ int x=2; { x=3; } x; })); // 138

    ASSERT(1, ({ int x; int y; char z; char *a=&y; char *b=&z; a-b; })); // 139
    ASSERT(4, ({ int x; char y; int z; char *a=&y; char *b=&z; a-b; })); // 140

    ASSERT(0, ""[0]);      // 141
    ASSERT(1, sizeof("")); // 142

    ASSERT(97, "abc"[0]);     // 143
    ASSERT(98, "abc"[1]);     // 144
    ASSERT(99, "abc"[2]);     // 145
    ASSERT(0, "abc"[3]);      // 146
    ASSERT(4, sizeof("abc")); // 147

    /*
    ASSERT(7, "\a"[0]);
    ASSERT(8, "\b"[0]);
    ASSERT(9, "\t"[0]);
    ASSERT(10, "\n"[0]);
    ASSERT(11, "\v"[0]);
    ASSERT(12, "\f"[0]);
    ASSERT(13, "\r"[0]);
    ASSERT(27, "\e"[0]);

    ASSERT(106, "\j"[0]);
    ASSERT(107, "\k"[0]);
    ASSERT(108, "\l"[0]);

    ASSERT(7, "\ax\ny"[0]);
    ASSERT(120, "\ax\ny"[1]);
    ASSERT(10, "\ax\ny"[2]);
    ASSERT(121, "\ax\ny"[3]);

    ASSERT(0, "\0"[0]);
    ASSERT(16, "\20"[0]);
    ASSERT(65, "\101"[0]);
    ASSERT(104, "\1500"[0]);
    ASSERT(0, "\x00"[0]);
    ASSERT(119, "\x77"[0]);
    */

    if (error_num == 0)
        printf("OK\n");
    else
        printf("\nthe number of error is %d\n", error_num);
    return 0;
}

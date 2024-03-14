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
    printf("\n\n\x1b[34mstring.c:\x1b[0m \n");
    num = 1;
    error_num = 0;

    ASSERT(0, ""[0]);      // 1
    ASSERT(1, sizeof("")); // 2

    ASSERT(97, "abc"[0]);     // 3
    ASSERT(98, "abc"[1]);     // 4
    ASSERT(99, "abc"[2]);     // 5
    ASSERT(0, "abc"[3]);      // 6
    ASSERT(4, sizeof("abc")); // 7

    /*
    ASSERT(7, "\a"[0]);//8
    ASSERT(8, "\b"[0]);//9
    ASSERT(9, "\t"[0]);//10
    ASSERT(10, "\n"[0]);//11
    ASSERT(11, "\v"[0]);//12
    ASSERT(12, "\f"[0]);//13
    ASSERT(13, "\r"[0]);//14
    ASSERT(27, "\e"[0]);//15

    ASSERT(106, "\j"[0]);//16
    ASSERT(107, "\k"[0]);//17
    ASSERT(108, "\l"[0]);//18

    ASSERT(7, "\ax\ny"[0]);//19
    ASSERT(120, "\ax\ny"[1]);//20
    ASSERT(10, "\ax\ny"[2]);//21
    ASSERT(121, "\ax\ny"[3]);//22

    ASSERT(0, "\0"[0]);//23
    ASSERT(16, "\20"[0]);//24
    ASSERT(65, "\101"[0]);//25
    ASSERT(104, "\1500"[0]);//26
    ASSERT(0, "\x00"[0]);//27
    ASSERT(119, "\x77"[0]);//28
    */

    if (error_num == 0)
        printf("OK\n");
    else
        printf("\nthe number of error is %d\n", error_num);
    return 0;
}

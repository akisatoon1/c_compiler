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
    printf("\n\n\x1b[34marith.c:\x1b[0m \n");
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

    ASSERT(0, 0 && 1);       // 27
    ASSERT(0, (2 - 2) && 5); // 28
    ASSERT(1, 1 && 5);       // 29

    if (error_num == 0)
        printf("OK\n");
    else
        printf("\nthe number of error is %d\n", error_num);
    return 0;
}

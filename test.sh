#!/bin/bash
test[0]="./test/arith.c"
test[1]="./test/control.c"
test[2]="./test/function.c"
test[3]="./test/pointer.c"
test[4]="./test/string.c"
test[5]="./test/struct.c"
test[6]="./test/variable.c"
for file in ${test[@]}
do
    ./9cc ${file} > tmp.s
    gcc -c ./tmp.s
    gcc tmp.o link.o -o tmp
    ./tmp
done

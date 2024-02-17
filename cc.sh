#!/bin/bash
./9cc './test/cc.c' > tmp.s
gcc -c ./tmp.s ./link/link.c
gcc tmp.o link.o -o tmp
./tmp
echo 'echo $? ->' $?

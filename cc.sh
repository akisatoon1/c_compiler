#!/bin/bash
./9cc './test/link.c' > tmp.s
gcc -c ./tmp.s ./link/link.c
gcc tmp.o link.o -o tmp
./tmp
echo 'echo $? ->' $?

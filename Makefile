CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc link.o
	./test.sh

link.o: ./link/link.c
	gcc -c ./link/link.c

cc: 9cc
	./cc.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean cc

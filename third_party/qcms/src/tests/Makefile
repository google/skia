CC=gcc
INCLUDE=-I../
WALL=-Wall
CFLAGS=-O2 -msse2 $(WALL) -DSSE2_ENABLE
LDFLAGS=-lm

QCMS=../transform.c ../transform-sse2.c ../transform_util.c ../matrix.c ../iccread.c ../chain.c ../qcms_util.c
OBJS=$(QCMS:.c=.o)

all: qcms_tests

qcms_tests: qcms_test_*.c $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf qcms_tests *.o $(OBJS)

CC = gcc
CFLAGS = -Wall -g -Iinclude -IstudentInclude

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	CCFLAGS += -std=gnu11 
endif
ifeq ($(UNAME), Darwin)
	CCFLAGS += -std=c11
endif

listTests:  
	$(CC)  $(CFLAGS) src/listTest.c src/testCases.c src/testharness.c studentCode/*.c  -o bin/listTests

calTests:  
	$(CC)  $(CFLAGS) src/calTest.c src/calTestCases.c src/testharness.c studentCode/*.c  -o bin/calTests

calTestsConst:  
	$(CC)  $(CFLAGS) -DCONST_ERR=0 src/calTest.c src/calTestCases.c src/testharness.c studentCode/*.c  -o bin/calTestsConst

calMemTest:
	$(CC)  $(CFLAGS) src/calMemTest.c studentCode/*.c  -o bin/calMemTest1
	$(CC)  $(CFLAGS) src/calMemTestPrint.c studentCode/*.c  -o bin/calMemTest2

calMemTestConst:
	$(CC)  $(CFLAGS) -DCONST_ERR=0 src/calMemTest.c studentCode/*.c  -o bin/calMemTestConst1
	$(CC)  $(CFLAGS) -DCONST_ERR=0 src/calMemTestPrint.c studentCode/*.c  -o bin/calMemTestConst2
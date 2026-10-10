CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2

all: pipetool

pipetool: src/pipetool.c
	$(CC) $(CFLAGS) -o $@ $<

pipetool_bug: experiments/pipetool_bug.c
	$(CC) $(CFLAGS) -o $@ $<

test: pipetool
	./tests/run_tests.sh

clean:
	rm -f pipetool pipetool_bug

.PHONY: all test clean

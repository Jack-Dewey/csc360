.phony all:
all: acs

acs: main.c queue.c
	gcc -w -pthread main.c queue.c -o ACS

.PHONY clean:
clean:
	-rm -rf *.o *.exe

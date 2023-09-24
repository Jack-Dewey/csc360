.phony all:
all: pman

pman: main.c linked_list.c
	gcc -w -Wall main.c -o pman

.PHONY clean:
clean:
	-rm -rf *.o *.exe

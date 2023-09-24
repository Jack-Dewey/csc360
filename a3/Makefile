.phony all:
all: diskinfo disklist diskget diskput

diskinfo: diskinfo.c
	gcc -w -Wall diskinfo.c -o diskinfo

disklist: disklist.c
	gcc -w -Wall disklist.c -o disklist

diskget: diskget.c
	gcc -w -Wall diskget.c -o diskget

diskput: diskput.c
	gcc -w -Wall diskput.c -o diskput

.PHONY clean:
clean:
	-rm -rf *.o *.exe

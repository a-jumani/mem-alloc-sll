mem:
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o

test:
	gcc -L. -o memtest memtest.c -Wall -Werror -lmem

clean:
	rm mem.o libmem.so memtest

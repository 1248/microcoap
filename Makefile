all:
	gcc -Wall -o coap main-posix.c coap.c -DDEBUG

clean:
	rm -f coap

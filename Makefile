all:
	gcc -Wall -o coap endpoints.c main-posix.c coap.c -DDEBUG

clean:
	rm -f coap

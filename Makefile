all:
	gcc -Wall -o coap main-posix.c coap.c endpoints.c -DDEBUG

clean:
	rm -f coap

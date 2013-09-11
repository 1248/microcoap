all:
	gcc -Wall -o coap main-posix.c coap.c endpoints.c

clean:
	rm -f coap

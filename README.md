microcoap
=========

A toy CoAP server for microcontrollers.
See http://tools.ietf.org/html/draft-ietf-core-coap-18

Endpoint handlers are defined in endpoints.c

 * Arduino demo
 * POSIX (OS X/Linux) demo
 * GET/PUT/POST

 * No retries
 * Piggybacked ACK only


For linux/OSX

    make
    ./coap

For Arduino

    open microcoap.ino

To test, use libcoap

    ./coap-client -v 100 -m get coap://127.0.0.1/.well-known/core
    ./coap-client -v 100 -m get coap://127.0.0.1/hello

Or use copper (Firefox plugin)

    coap://127.0.0.1

Arduino problem
===============

Arduino, by default, has a UDP transmit buffer of 24 bytes. This is too small
for some endpoints and will result in an error.


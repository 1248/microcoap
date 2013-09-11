microcoap
=========

A miminal CoAP server implementation for microcontrollers.
Written from the RFC, http://tools.ietf.org/html/draft-ietf-core-coap-18
Endpoint handlers are defined in endpoints.c

 * Arduino demo
 * POSIX (OS X/Linux) demo
 * Probably not compliant
 * Only supports GET
 * No retry logic

For linux/OSX

    make
    ./coap

For Arduino

    open microcoap.ino

To test, use libcoap

    ./coap-client -v 100 -m get coap://127.0.0.1/.well-known/core
    ./coap-client -v 100 -m get coap://127.0.0.1/hello


Arduino problem
===============

Arduino, by default, has a UDP transmit buffer of 24 bytes. This is too small
for some endpoints and will result in an error.


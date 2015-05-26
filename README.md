microcoap
=========

A tiny CoAP server for microcontrollers.
See http://tools.ietf.org/html/rfc7252

Endpoint handlers are defined in endpoints.c

 * Arduino demo (Mega + Ethernet shield, LED + 220R on pin 6, PUT "0" or "1" to /light)
 * POSIX (OS X/Linux) demo
 * GET/PUT/POST
 * No retries
 * Piggybacked ACK only


For linux/OSX

    make
    ./micocoap

For Arduino

    copy libraries and microcoap-example directories to the Arduino IDE sketches folder (or just change the sketches folder to the microcoap dir)
    open: File -> sketches folder -> microcoap-example

Arduino LED pin defined at `microcoap-example\connections.h` and set to `13` (default embedded led for some Arduino boards)

To test, use libcoap

    ./coap-client -v 100 -m get coap://127.0.0.1/.well-known/core
    ./coap-client -v 100 -m get coap://127.0.0.1/light
    ./coap-client -e "1" -m put coap://127.0.0.1/light
    ./coap-client -e "0" -m put coap://127.0.0.1/light

Or use copper (Firefox plugin)

    coap://127.0.0.1
    
How to add new coap links
======================

Use `microcoap-example` dir and edit it or prepare similar folder.
Edit `endpoints.h` and `endpoints.c` similar to light entry.

Arduino problem
===============

Arduino, by default, has a UDP transmit buffer of 24 bytes. This is too small
for some endpoints and will result in an error.


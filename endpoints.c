#include <stdbool.h>
#include <strings.h>
#include "coap.h"

static const coap_endpoint_path_t path_well_known_core = {2, {".well-known", "core"}};
static int handle_get_well_known_core(const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    static char *rsp = "</hello>;title=\"Hi\"";
    coap_make_response(outpkt, (const uint8_t *)rsp, strlen(rsp), id_hi, id_lo, COAP_RSPCODE_CONTENT);
    return 0;
}

static const coap_endpoint_path_t path_hello = {1, {"hello"}};
static int handle_get_hello(const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    static char *rsp = "Hello World";
    coap_make_response(outpkt, (const uint8_t *)rsp, strlen(rsp), id_hi, id_lo, COAP_RSPCODE_CONTENT);
    return 0;
}

static int handle_put_hello(const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    uint8_t rspbuf[32];
    size_t len;
    // respond with "Hello " + payload
    memcpy(rspbuf, "Hello ", 6);
    len = inpkt->payload.len;
    if (len + 6 + 1 > sizeof(rspbuf))
        len = sizeof(rspbuf) - (6 + 1);
    memcpy(rspbuf + 6, inpkt->payload.p, len);
    rspbuf[len + 6] = 0;

    coap_make_response(outpkt, (const uint8_t *)rspbuf, len + 6 + 1, id_hi, id_lo, COAP_RSPCODE_CONTENT);
    return 0;
}


const coap_endpoint_t endpoints[] =
{
    {COAP_METHOD_GET, handle_get_well_known_core, &path_well_known_core},
    {COAP_METHOD_GET, handle_get_hello, &path_hello},
    {COAP_METHOD_PUT, handle_put_hello, &path_hello},
    {0, NULL, NULL}
};



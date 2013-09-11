#include <stdbool.h>
#include <strings.h>

#include "coap.h"

static const coap_endpoint_path_t path_well_known_core = {2, {".well-known", "core"}};
static int handle_get_well_known_core(coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    static char *rsp = "</hello>;title=\"Hi\"";
    coap_make_get_response(outpkt, (const uint8_t *)rsp, strlen(rsp), id_hi, id_lo);
    return 0;
}

static const coap_endpoint_path_t path_hello = {1, {"hello"}};
static int handle_get_hello(coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    static char *rsp = "Hello World";
    coap_make_get_response(outpkt, (const uint8_t *)rsp, strlen(rsp), id_hi, id_lo);
    return 0;
}

const coap_endpoint_t endpoints[] =
{
    {COAP_METHOD_GET, handle_get_well_known_core, &path_well_known_core},
    {COAP_METHOD_GET, handle_get_hello, &path_hello},
    {0, NULL, NULL}
};



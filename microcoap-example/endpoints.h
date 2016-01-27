#ifndef ENDPOINTS_H
#define ENDPOINTS_H 1
#ifdef __cplusplus
extern "C" {
#endif

#include "microcoap.h"

static int handle_get_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);
static int handle_put_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);

#ifdef __cplusplus
}
#endif
#endif // ENDPOINTS_H

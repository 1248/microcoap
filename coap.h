#ifndef COAP_H
#define COAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAXOPT 16

// http://tools.ietf.org/html/draft-ietf-core-coap-18#section-3
typedef struct
{
    uint8_t ver;
    uint8_t t;
    uint8_t tkl;
    uint8_t code;
    uint8_t id[2];
} coap_header_t;

typedef struct
{
    const uint8_t *p;
    size_t len;
} coap_buffer_t;

typedef struct
{
    uint8_t *p;
    size_t len;
} coap_rw_buffer_t;

typedef struct
{
    uint8_t num;
    coap_buffer_t buf;
} coap_option_t;

typedef struct
{
    coap_header_t hdr;
    coap_buffer_t tok;
    uint8_t numopts;
    coap_option_t opts[MAXOPT];
    coap_buffer_t payload;
} coap_packet_t;

/////////////////////////////////////////

//http://tools.ietf.org/html/draft-ietf-core-coap-18#section-12.2
typedef enum
{
    COAP_OPTION_IF_MATCH = 1,
    COAP_OPTION_URI_HOST = 3,
    COAP_OPTION_ETAG = 4,
    COAP_OPTION_IF_NONE_MATCH = 5,
    COAP_OPTION_URI_PORT = 7,
    COAP_OPTION_LOCATION_PATH = 8,
    COAP_OPTION_URI_PATH = 11,
    COAP_OPTION_CONTENT_FORMAT = 12,
    COAP_OPTION_MAX_AGE = 14,
    COAP_OPTION_URI_QUERY = 15,
    COAP_OPTION_ACCEPT = 17,
    COAP_OPTION_LOCATION_QUERY = 20,
    COAP_OPTION_PROXY_URI = 35,
    COAP_OPTION_PROXY_SCHEME = 39
} coap_option_num_t;

//http://tools.ietf.org/html/draft-ietf-core-coap-18#section-12.1.1
typedef enum
{
    COAP_METHOD_GET = 1,
    COAP_METHOD_POST = 2,
    COAP_METHOD_PUT = 3,
    COAP_METHOD_DELETE = 4
} coap_method_t;

//http://tools.ietf.org/html/draft-ietf-core-coap-18#section-12.1.1
typedef enum
{
    COAP_TYPE_CON = 0,
    COAP_TYPE_NONCON = 1,
    COAP_TYPE_ACK = 2,
    COAP_TYPE_RESET = 3
} coap_msgtype_t;

//http://tools.ietf.org/html/draft-ietf-core-coap-18#section-5.2
//http://tools.ietf.org/html/draft-ietf-core-coap-18#section-12.1.2
#define MAKE_RSPCODE(clas, det) ((clas << 5) | (det))
typedef enum
{
    COAP_RSPCODE_CONTENT = MAKE_RSPCODE(2, 5),
    COAP_RSPCODE_NOT_FOUND = MAKE_RSPCODE(4, 4),
    COAP_RSPCODE_BAD_REQUEST = MAKE_RSPCODE(4, 0),
    COAP_RSPCODE_CHANGED = MAKE_RSPCODE(2, 4)
} coap_responsecode_t;

//http://tools.ietf.org/html/draft-ietf-core-coap-18#section-12.3
typedef enum
{
    COAP_CONTENTTYPE_NONE = -1, // bodge to allow us not to send option block
    COAP_CONTENTTYPE_TEXT_PLAIN = 0,
    COAP_CONTENTTYPE_APPLICATION_LINKFORMAT = 40,
} coap_content_type_t;

///////////////////////

typedef enum
{
    COAP_ERR_NONE = 0,
    COAP_ERR_HEADER_TOO_SHORT = 1,
    COAP_ERR_VERSION_NOT_1 = 2,
    COAP_ERR_TOKEN_TOO_SHORT = 3,
    COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER = 4,
    COAP_ERR_OPTION_TOO_SHORT = 5,
    COAP_ERR_OPTION_OVERRUNS_PACKET = 6,
    COAP_ERR_OPTION_TOO_BIG = 7,
    COAP_ERR_OPTION_LEN_INVALID = 8,
    COAP_ERR_BUFFER_TOO_SMALL = 9,
    COAP_ERR_UNSUPPORTED = 10,
    COAP_ERR_OPTION_DELTA_INVALID = 11,
} coap_error_t;

///////////////////////

typedef int (*coap_endpoint_func)(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);
#define MAX_SEGMENTS 2  // 2 = /foo/bar, 3 = /foo/bar/baz
typedef struct
{
    int count;
    const char *elems[MAX_SEGMENTS];
} coap_endpoint_path_t;

typedef struct
{
    coap_method_t method;
    coap_endpoint_func handler;
    const coap_endpoint_path_t *path;
} coap_endpoint_t;


///////////////////////
void coap_dumpPacket(coap_packet_t *pkt);
int coap_parse(coap_packet_t *pkt, const uint8_t *buf, size_t buflen);
int coap_buffer_to_string(char *strbuf, size_t strbuflen, const coap_buffer_t *buf);
const coap_option_t *coap_findOptions(const coap_packet_t *pkt, uint8_t num, uint8_t *count);
int coap_build(uint8_t *buf, size_t *buflen, const coap_packet_t *pkt);
void coap_dump(const uint8_t *buf, size_t buflen, bool bare);
int coap_make_response(coap_rw_buffer_t *scratch, coap_packet_t *pkt, const uint8_t *content, size_t content_len, uint8_t msgid_hi, uint8_t msgid_lo, coap_responsecode_t rspcode, coap_content_type_t content_type);
int coap_handle_req(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt);
void coap_setup(void);
void endpoint_setup(void);

#ifdef __cplusplus
}
#endif

#endif

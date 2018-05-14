#ifndef COAP_H
#define COAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAXOPT 16

//http://tools.ietf.org/html/rfc7252#section-3
typedef struct
{
    uint8_t ver;                /* CoAP version number */
    uint8_t t;                  /* CoAP Message Type */
    uint8_t tkl;                /* Token length: indicates length of the Token field */
    uint8_t code;               /* CoAP status code. Can be request (0.xx), success reponse (2.xx), 
                                 * client error response (4.xx), or rever error response (5.xx) 
                                 * For possible values, see http://tools.ietf.org/html/rfc7252#section-12.1 */
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
    uint8_t num;                /* Option number. See http://tools.ietf.org/html/rfc7252#section-5.10 */
    coap_buffer_t buf;          /* Option value */
} coap_option_t;

typedef struct
{
    coap_header_t hdr;          /* Header of the packet */
    coap_buffer_t tok;          /* Token value, size as specified by hdr.tkl */
    uint8_t numopts;            /* Number of options */
    coap_option_t opts[MAXOPT]; /* Options of the packet. For possible entries see
                                 * http://tools.ietf.org/html/rfc7252#section-5.10 */
    coap_buffer_t payload;      /* Payload carried by the packet */
} coap_packet_t;

/////////////////////////////////////////

//http://tools.ietf.org/html/rfc7252#section-12.2
typedef enum
{
    COAP_OPTION_IF_MATCH = 1,
    COAP_OPTION_URI_HOST = 3,
    COAP_OPTION_ETAG = 4,
    COAP_OPTION_IF_NONE_MATCH = 5,
    COAP_OPTION_OBSERVE = 6,
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

//http://tools.ietf.org/html/rfc7252#section-12.1.1
typedef enum
{
    COAP_METHOD_GET = 1,
    COAP_METHOD_POST = 2,
    COAP_METHOD_PUT = 3,
    COAP_METHOD_DELETE = 4
} coap_method_t;

//http://tools.ietf.org/html/rfc7252#section-12.1.1
typedef enum
{
    COAP_TYPE_CON = 0,
    COAP_TYPE_NONCON = 1,
    COAP_TYPE_ACK = 2,
    COAP_TYPE_RESET = 3
} coap_msgtype_t;

//http://tools.ietf.org/html/rfc7252#section-5.2
//http://tools.ietf.org/html/rfc7252#section-12.1.2
#define MAKE_RSPCODE(clas, det) ((clas << 5) | (det))
typedef enum
{
    COAP_RSPCODE_CONTENT = MAKE_RSPCODE(2, 5),
    COAP_RSPCODE_NOT_FOUND = MAKE_RSPCODE(4, 4),
    COAP_RSPCODE_BAD_REQUEST = MAKE_RSPCODE(4, 0),
    COAP_RSPCODE_CHANGED = MAKE_RSPCODE(2, 4)
} coap_responsecode_t;

// https://www.iana.org/assignments/core-parameters/core-parameters.xhtml#content-formats
// http://tools.ietf.org/html/rfc7252#section-12.3
typedef enum
{
    COAP_CONTENTTYPE_NONE = -1, // bodge to allow us not to send option block

    /* 0-255  Expert Review */
    COAP_CONTENTTYPE_TEXT_PLAIN                   = 0    ,  //  text/plain; charset=utf-8                    /* Ref: [RFC2046][RFC3676][RFC5147] */
    COAP_CONTENTTYPE_APPLICATION_COSE_ENCRYPT0    = 16   ,  //  application/cose; cose-type="cose-encrypt0"  /* Ref: [RFC8152] */
    COAP_CONTENTTYPE_APPLICATION_COSE_MAC0        = 17   ,  //  application/cose; cose-type="cose-mac0"      /* Ref: [RFC8152] */
    COAP_CONTENTTYPE_APPLICATION_COSE_SIGN1       = 18   ,  //  application/cose; cose-type="cose-sign1"     /* Ref: [RFC8152] */
    COAP_CONTENTTYPE_APPLICATION_LINKFORMAT       = 40   ,  //  application/link-format                      /* Ref: [RFC6690] */
    COAP_CONTENTTYPE_APPLICATION_XML              = 41   ,  //  application/xml                              /* Ref: [RFC3023] */
    COAP_CONTENTTYPE_APPLICATION_OCTECT_STREAM    = 42   ,  //  application/octet-stream                     /* Ref: [RFC2045][RFC2046] */
    COAP_CONTENTTYPE_APPLICATION_EXI              = 47   ,  //  application/exi                              /* Ref: ["Efficient XML Interchange (EXI) Format 1.0 (Second Edition)" ,February 2014] */
    COAP_CONTENTTYPE_APPLICATION_JSON             = 50   ,  //  application/json                             /* Ref: [RFC4627] */
    COAP_CONTENTTYPE_APPLICATION_JSON_PATCH_JSON  = 51   ,  //  application/json-patch+json                  /* Ref: [RFC6902] */
    COAP_CONTENTTYPE_APPLICATION_MERGE_PATCH_JSON = 52   ,  //  application/merge-patch+json                 /* Ref: [RFC7396] */
    COAP_CONTENTTYPE_APPLICATION_CBOR             = 60   ,  //  application/cbor                             /* Ref: [RFC7049] */
    COAP_CONTENTTYPE_APPLICATION_CWT              = 61   ,  //  application/cwt                              /* Ref: [RFC8392] */
    COAP_CONTENTTYPE_APPLICATION_COSE_ENCRYPT     = 96   ,  //  application/cose; cose-type="cose-encrypt"   /* Ref: [RFC8152] */
    COAP_CONTENTTYPE_APPLICATION_COSE_MAC         = 97   ,  //  application/cose; cose-type="cose-mac"       /* Ref: [RFC8152] */
    COAP_CONTENTTYPE_APPLICATION_COSE_SIGN        = 98   ,  //  application/cose; cose-type="cose-sign"      /* Ref: [RFC8152] */
    COAP_CONTENTTYPE_APPLICATION_COSE_KEY         = 101  ,  //  application/cose-key                         /* Ref: [RFC8152] */
    COAP_CONTENTTYPE_APPLICATION_COSE_KEY_SET     = 102  ,  //  application/cose-key-set                     /* Ref: [RFC8152] */
    COAP_CONTENTTYPE_APPLICATION_COAP_GROUP_JSON  = 256  ,  //  application/coap-group+json                  /* Ref: [RFC7390] */
    /* 256-9999  IETF Review or IESG Approval */
    COAP_CONTENTTYPE_APPLICATION_OMA_TLV_OLD      = 1542 ,  //  Keep old value for backward-compatibility    /* Ref: [OMA-TS-LightweightM2M-V1_0] */
    COAP_CONTENTTYPE_APPLICATION_OMA_JSON_OLD     = 1543 ,  //  Keep old value for backward-compatibility    /* Ref: [OMA-TS-LightweightM2M-V1_0] */
    /* 10000-64999  First Come First Served */
    COAP_CONTENTTYPE_APPLICATION_VND_OCF_CBOR     = 10000,  //  application/vnd.ocf+cbor                     /* Ref: [Michael_Koster] */
    COAP_CONTENTTYPE_APPLICATION_OMA_TLV          = 11542,  //  application/vnd.oma.lwm2m+tlv                /* Ref: [OMA-TS-LightweightM2M-V1_0] */
    COAP_CONTENTTYPE_APPLICATION_OMA_JSON         = 11543,  //  application/vnd.oma.lwm2m+json               /* Ref: [OMA-TS-LightweightM2M-V1_0] */
    /* 65000-65535  Experimental use (no operational use) */
}

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
    coap_method_t method;               /* (i.e. POST, PUT or GET) */
    coap_endpoint_func handler;         /* callback function which handles this 
                                         * type of endpoint (and calls 
                                         * coap_make_response() at some point) */
    const coap_endpoint_path_t *path;   /* path towards a resource (i.e. foo/bar/) */ 
    const char *core_attr;              /* the 'ct' attribute, as defined in RFC7252, section 7.2.1.:
                                         * "The Content-Format code "ct" attribute 
                                         * provides a hint about the 
                                         * Content-Formats this resource returns." 
                                         * (Section 12.3. lists possible ct values.) */
} coap_endpoint_t;


///////////////////////
void coap_dumpPacket(coap_packet_t *pkt);
int coap_parse(coap_packet_t *pkt, const uint8_t *buf, size_t buflen);
int coap_buffer_to_string(char *strbuf, size_t strbuflen, const coap_buffer_t *buf);
const coap_option_t *coap_findOptions(const coap_packet_t *pkt, uint8_t num, uint8_t *count);
int coap_build(uint8_t *buf, size_t *buflen, const coap_packet_t *pkt);
void coap_dump(const uint8_t *buf, size_t buflen, bool bare);
int coap_make_response(coap_rw_buffer_t *scratch, coap_packet_t *pkt, const uint8_t *content, size_t content_len, uint8_t msgid_hi, uint8_t msgid_lo, const coap_buffer_t* tok, coap_responsecode_t rspcode, coap_content_type_t content_type);
int coap_handle_req(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt);
void coap_option_nibble(uint32_t value, uint8_t *nibble);
void coap_setup(void);
void endpoint_setup(void);

#ifdef __cplusplus
}
#endif

#endif

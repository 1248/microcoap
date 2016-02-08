/**
 * @file coap.h
 * 
 * @brief A tiny CoAP library for microcontrollers.
 * 
 * @mainpage microcoap
 * 
 * A tiny CoAP server for microcontrollers.
 * 
 * See [RFC 7252](http://tools.ietf.org/html/rfc7252).
 * 
 * Endpoint handlers are defined in endpoints.c
 * 
 *  * Arduino demo (Mega + Ethernet shield, LED + 220R on pin 6, PUT "0" or "1" to /light)
 *  * POSIX (OS X/Linux) demo
 *  * GET/PUT/POST
 *  * No retries
 *  * Piggybacked ACK only
 * 
 * 
 * For linux/OSX
 * 
 *     make
 *     ./coap
 * 
 * For Arduino
 * 
 *     open microcoap.ino
 * 
 * To test, use libcoap
 * 
 *     ./coap-client -v 100 -m get coap://127.0.0.1/.well-known/core
 *     ./coap-client -v 100 -m get coap://127.0.0.1/light
 *     ./coap-client -e "1" -m put coap://127.0.0.1/light
 *     ./coap-client -e "0" -m put coap://127.0.0.1/light
 * 
 * Or use copper (Firefox plugin)
 * 
 *     coap://127.0.0.1
 * 
 * Arduino problem
 * ===============
 * 
 * Arduino, by default, has a UDP transmit buffer of 24 bytes. This is too small
 * for some endpoints and will result in an error.
 * 
 * @author Toby Jaffey <toby@1248.io>
 */

#ifndef COAP_H
#define COAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAXOPT 16  //!< The maximum number of options supported in one packet.

//http://tools.ietf.org/html/rfc7252#section-3
typedef struct
{
    uint8_t ver;    //!< version number
    uint8_t t;      //!< message type
    uint8_t tkl;    //!< token length
    uint8_t code;   //!< status code Can be request (0.xx), success reponse (2.xx), 
                    //!< client error response (4.xx), or server error response (5.xx)
    uint8_t id[2];  //!< message ID
} coap_header_t;

typedef struct
{
    const uint8_t *p;  //!< byte array that holds some data, immutable
    size_t len;        //!< length of the array
} coap_buffer_t;

typedef struct
{
    uint8_t *p;  //!< byte array that holds some data, mutable
    size_t len;  //!< length of the array
} coap_rw_buffer_t;

typedef struct
{
    uint8_t num;        //!< option number
    coap_buffer_t buf;  //!< option value
} coap_option_t;

typedef struct
{
    coap_header_t hdr;           //!< header of the packet
    coap_buffer_t tok;           //!< token value, size as specified by header.tkl
    uint8_t numopts;             //!< number of options
    coap_option_t opts[MAXOPT];  //!< options of the packet
    coap_buffer_t payload;       //!< payload carried by the packet
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

//http://tools.ietf.org/html/rfc7252#section-12.3
typedef enum
{
    COAP_CONTENTTYPE_NONE = -1, // bodge to allow us not to send option block
    COAP_CONTENTTYPE_TEXT_PLAIN = 0,
    COAP_CONTENTTYPE_APPLICATION_LINKFORMAT = 40,
    COAP_CONTENTTYPE_APPLICATION_XML = 41,
    COAP_CONTENTTYPE_APPLICATION_OCTECT_STREAM = 42,
    COAP_CONTENTTYPE_APPLICATION_EXI = 47,
    COAP_CONTENTTYPE_APPLICATION_JSON = 50,
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
#define MAX_SEGMENTS 2  //!< Maximum number of URI segments supported (e.g. 2 = /foo/bar, 3 = /foo/bar/baz)
typedef struct
{
    int count;                        //!< Number of segments (i.e. number of elements in \p elems)
    const char *elems[MAX_SEGMENTS];  //!< Array containing pointers to the segments
} coap_endpoint_path_t;

typedef struct
{
    coap_method_t method;               //!< Request method (GET, POST, PUT, or DELETE)
    coap_endpoint_func handler;         //!< callback function which handles this type of
                                        //!< endpoint (and calls coap_make_response() at some point)
    const coap_endpoint_path_t *path;   //!< path towards a resource (e.g. /foo/bar)
    const char *core_attr;              //!< the 'ct' attribute, as defined in RFC7252, section 7.2.1.
} coap_endpoint_t;


///////////////////////

/**
 * Dumps all values of a CoAP packet (including payload) as hexadecimal.
 * 
 * @param[in] pkt Pointer to the packet whose content is to be dumped.
 */
void coap_dumpPacket(coap_packet_t *pkt);

/**
 * Parses the content of \p buf (i.e. the content of a UDP packet) and
 * writes the values to \p pkt.
 * 
 * @param[out] pkt The coap_packet_t structure to be filled.
 * @param[in] buf The buffer containing the CoAP packet in binary format.
 * @param[in] buflen The lenth of \p buf in bytes.
 * 
 * @return 0 on success, or the according coap_error_t
 */
int coap_parse(coap_packet_t *pkt, const uint8_t *buf, size_t buflen);

/**
 * Converts the data in \p buf into a null-terminated C string and
 * copies the result to \p strbuf.
 * 
 * @param[out] strbuf The char buffer to which the content of \p buf will be
 * written to.
 * @param[in] strbuflen The length of \p strbuf, to prevent overflows.
 * @param[in] buf The coap_buffer_t structure whose content is to be converted.
 * 
 * @return 0 on success, or COAP_ERR_BUFFER_TOO_SMALL if \p strbuflen
 * is smaller than the size of \p buf.
 */
int coap_buffer_to_string(char *strbuf, size_t strbuflen, const coap_buffer_t *buf);

/**
 * Finds the position of the option with number num in \p pkt, and stores the
 * number of of occurence in the packet in \p count.
 * 
 * @param[in] pkt The coap_packet_t structure containing the options.
 * @param[in] num The option number as defined in RFC7252.
 * @param[out] count Stores how often the option specified by \p num occurs
 * in \p pkt.
 * 
 * @return A pointer to a coap_option_t structure, or NULL if the packet
 * contains no such option.
 */
const coap_option_t *coap_findOptions(const coap_packet_t *pkt, uint8_t num, uint8_t *count);

/**
 * Creates a CoAP message from the data in \p pkt and writes the
 * result to \p buf. The actual size of the whole message (which
 * may be smaller than the size of the buffer) will be written to
 * \p buflen. You should use that value (and not \p buflen)
 * when you send the message.
 * 
 * @param[out] buf Byte buffer to which the CoAP packet in binary format will
 * be written to.
 * @param[in,out] buflen Contains the initial size of \p buf, then stores how
 * many bytes have been written to \p buf.
 * @param[in] pkt The packet that is to be converted to binary format. 
 * 
 * @return 0 on success, or COAP_ERR_BUFFER_TOO_SMALL if the size of
 * \p buf is not sufficient, or COAP_ERR_UNSUPPORTED if
 * the token length specified in the header does not match the
 * token length specified in the buffer that actually holds the
 * tokens
 */
int coap_build(uint8_t *buf, size_t *buflen, const coap_packet_t *pkt);

/**
 * Dumps the content of \p buf as hexadecimal.
 * 
 * @param[in] buf The buffer to be dumped.
 * @param[in] buflen The length of \p buf in bytes.
 * @param[in] bare If true, "Dump:" and a the newline character are printed
 * before the actual values.
 */
void coap_dump(const uint8_t *buf, size_t buflen, bool bare);

/**
 * Creates a response packet to a request, including a piggybacked ACK for
 * the request packet, and stores it in \p pkt.
 * 
 * @param[out] scratch Buffer for the content type number.
 * @param[out] pkt Pointer to the coap_packet_t that will be filled.
 * @param[in] content The response payload.
 * @param[in] content_len Length of \p content in bytes.
 * @param[in] msgid_hi The high byte of the message ID.
 * @param[in] msgid_lo The low byte of the message ID.
 * @param[in] tok Pointer to the token used.
 * @param[in] rspcode The response code.
 * @param[in] content_type The content type (i.e. what does the payload contain)
 * 
 * @return 0 on success, or COAP_ERR_BUFFER_TOO_SMALL if the length of the
 * buffer pointed to by \p scratch is smaller than 2.
 */
int coap_make_response(coap_rw_buffer_t *scratch, coap_packet_t *pkt, const uint8_t *content, size_t content_len, uint8_t msgid_hi, uint8_t msgid_lo, const coap_buffer_t* tok, coap_responsecode_t rspcode, coap_content_type_t content_type);

/**
  * Handles the request in \p inpkt, and creates a response packet which is
  * stored in \p outpkt. The response will include a piggybacked ACK for the
  * request packet, and will be non-confirmable.
  * 
  * @param[out] scratch Buffer for the content type number.
  * @param[in] inpkt Pointer to the coap_packet_t structure containing the
  * request.
  * @param[out] outpkt Pointer to the coap_packet_t structure that will be
  * filled, then containing the response.
  * 
  * @return The return code of the corresponding handler function, or 0 if
  * no corresponding handler exists.
  */
int coap_handle_req(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt);

void coap_option_nibble(uint32_t value, uint8_t *nibble);

void coap_setup(void);

void endpoint_setup(void);

#ifdef __cplusplus
}
#endif

#endif

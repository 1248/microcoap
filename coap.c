#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "coap.h"

extern const coap_endpoint_t endpoints[];

typedef struct __attribute__((packed)) {
    uint8_t szx      : 3;
    uint8_t more     : 1;
    uint8_t num      : 4;
} coap_opt_block2_t;

typedef struct __attribute__((packed)) {
    uint8_t num2     : 8;
    uint8_t szx      : 3;
    uint8_t more     : 1;
    uint8_t num1     : 4;
} coap_opt_block2_lng_t;

typedef struct __attribute__((packed)) {
    uint8_t num3     : 8;
    uint8_t num2     : 8;
    uint8_t szx      : 3;
    uint8_t more     : 1;
    uint8_t num1     : 4;
} coap_opt_block3_lng_t;

typedef union {
    uint8_t raw;
    struct {
        uint8_t tkl     : 4;
        uint8_t t       : 2;
        uint8_t ver     : 2;
        uint8_t code;
        uint16_t id;
    } hdr;
} coap_raw_header_t;

#ifdef MICROCOAP_DEBUG
void coap_dumpHeader(coap_header_t *hdr)
{
    printf("Header:\n");
    printf("  ver  0x%02X\n", hdr->ver);
    printf("  t    0x%02X\n", hdr->t);
    printf("  tkl  0x%02X\n", hdr->tkl);
    printf("  code 0x%02X\n", hdr->code);
    printf("  id   0x%02X%02X\n", hdr->id[0], hdr->id[1]);
}
#endif

#ifdef MICROCOAP_DEBUG
void coap_dump(const uint8_t *buf, size_t buflen, bool bare)
{
    if (bare)
    {
        while(buflen--)
            printf("%02X%s", *buf++, (buflen > 0) ? " " : "");
    }
    else
    {
        printf("Dump: ");
        while(buflen--)
            printf("%02X%s", *buf++, (buflen > 0) ? " " : "");
        printf("\n");
    }
}
#endif

int coap_parseHeader(coap_header_t *hdr, const uint8_t *buf, size_t buflen)
{
    if (buflen < sizeof(coap_raw_header_t))
        return COAP_ERR_HEADER_TOO_SHORT;

    coap_raw_header_t *r = (coap_raw_header_t*)buf;
    hdr->ver = r->hdr.ver;
    hdr->t = r->hdr.t;
    hdr->tkl = r->hdr.tkl;
    hdr->code = r->hdr.code;
    hdr->id = r->hdr.id;

    if (hdr->ver != 1)
        return COAP_ERR_VERSION_NOT_1;

    return 0;
}

int coap_parseToken(coap_packet_t *pkt, const uint8_t *buf, size_t buflen)
{
    coap_buffer_t *tok = &(pkt->tok);
    int toklen = pkt->hdr.tkl;

    // validate the token length
    if (sizeof(coap_raw_header_t) + toklen > buflen || toklen > 8)
        return COAP_ERR_TOKEN_TOO_SHORT;

    tok->len = toklen;

    if (!toklen)
        tok->p = NULL;
    else
        tok->p = buf + sizeof(coap_raw_header_t);  // past header

    return 0;
}

// advances p
int coap_parseOption(coap_option_t *option, uint16_t *running_delta, const uint8_t **buf, size_t buflen)
{
    const uint8_t *p = *buf;
    uint8_t headlen = 1;
    uint16_t len, delta;

    if (buflen < headlen) // too small
        return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;

    delta = (p[0] & 0xF0) >> 4;
    len = p[0] & 0x0F;

    // These are untested and may be buggy
    if (delta == 13)
    {
        headlen++;
        if (buflen < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        delta = p[1] + 13;
        p++;
    }
    else if (delta == 14)
    {
        headlen += 2;
        if (buflen < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        delta = ((p[1] << 8) | p[2]) + 269;
        p+=2;
    }
    else if (delta == 15)
        return COAP_ERR_OPTION_DELTA_INVALID;

    if (len == 13)
    {
        headlen++;
        if (buflen < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        len = p[1] + 13;
        p++;
    }
    else if (len == 14)
    {
        headlen += 2;
        if (buflen < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;

        len = ((p[1] << 8) | p[2]) + 269;
        p+=2;
    }
    else if (len == 15)
        return COAP_ERR_OPTION_LEN_INVALID;

    if ((p + 1 + len) > (*buf + buflen))
        return COAP_ERR_OPTION_TOO_BIG;

    option->num = delta + *running_delta;
    option->buf.p = p+1;
    option->buf.len = len;

    // advance buf
    *buf = p + 1 + len;
    *running_delta += delta;

    return 0;
}

// http://tools.ietf.org/html/rfc7252#section-3.1
int coap_parseOptionsAndPayload(coap_packet_t *pkt, const uint8_t *buf, size_t buflen)
{
    size_t optionIndex = 0;
    uint16_t delta = 0;
    const uint8_t *p = buf + sizeof(coap_raw_header_t) + pkt->hdr.tkl;
    const uint8_t *end = buf + buflen;
    int rc;
    if (p > end)
        return COAP_ERR_OPTION_OVERRUNS_PACKET;   // out of bounds

    // 0xFF is payload marker
    while((optionIndex < MAXOPT) && (p < end) && (*p != 0xFF))
    {
        rc = coap_parseOption(&pkt->opts[optionIndex], &delta, &p, end - p);
        if (rc)
            return rc;

        optionIndex++;
    }

    pkt->numopts = optionIndex;

    // payload marker
    if ((p + 1) < end && *p == 0xFF)
    {
        pkt->payload.p = p + 1;
        pkt->payload.len = end - (p + 1);
    }
    else
    {
        pkt->payload.p = NULL;
        pkt->payload.len = 0;
    }

    return 0;
}

#ifdef MICROCOAP_DEBUG
void coap_dumpOptions(coap_option_t *opts, size_t numopt)
{
    size_t i;
    printf(" Options:\n");
    for (i=0;i<numopt;i++)
    {
        printf("  0x%02X [ ", opts[i].num);
        coap_dump(opts[i].buf.p, opts[i].buf.len, true);
        printf(" ]\n");
    }
}
#endif

#ifdef MICROCOAP_DEBUG
void coap_dumpPacket(coap_packet_t *pkt)
{
    coap_dumpHeader(&pkt->hdr);
    coap_dumpOptions(pkt->opts, pkt->numopts);
    printf("Payload: ");
    coap_dump(pkt->payload.p, pkt->payload.len, true);
    printf("\n");
}
#endif

int coap_parse(coap_packet_t *pkt, const uint8_t *buf, size_t buflen)
{
    int rc;

    rc = coap_parseHeader(&(pkt->hdr), buf, buflen);
    if (rc)
        return rc;

    rc = coap_parseToken(pkt, buf, buflen);
    if (rc)
        return rc;

    pkt->numopts = MAXOPT;
    rc = coap_parseOptionsAndPayload(pkt, buf, buflen);
    if (rc)
        return rc;

    return rc;
}

// options are always stored consecutively, so can return a block with same option num
const coap_option_t *coap_findOptions(const coap_packet_t *pkt, coap_option_num_t num, uint8_t *count)
{
    // FIXME, options is always sorted, can find faster than this
    const coap_option_t *first = NULL;
    size_t i;

    *count = 0;

    for (i = 0; i < pkt->numopts; ++i)
    {
        if (pkt->opts[i].num != num) {
            if (!first)
                continue;
            else
                break;
        }

        ++(*count);
        if (!first)
            first = &pkt->opts[i];
    }

    return first;
}

int coap_buffer_to_string(char *strbuf, size_t strbuflen, const coap_buffer_t *buf)
{
    if (buf->len+1 > strbuflen)
        return COAP_ERR_BUFFER_TOO_SMALL;

    memcpy(strbuf, buf->p, buf->len);
    strbuf[buf->len] = 0;
    return 0;
}

int coap_build(uint8_t *buf, size_t *buflen, const coap_packet_t *pkt)
{
    uint16_t running_delta = 0;
    size_t opts_len = 0;
    uint8_t *p;
    size_t i;

    // build header
    if (*buflen < (sizeof(coap_raw_header_t) + pkt->hdr.tkl))
        return COAP_ERR_BUFFER_TOO_SMALL;

    coap_raw_header_t *r = (coap_raw_header_t*)buf;
    r->hdr.ver = pkt->hdr.ver;
    r->hdr.t = pkt->hdr.t;
    r->hdr.tkl = pkt->hdr.tkl;
    r->hdr.code = pkt->hdr.code;
    r->hdr.id = pkt->hdr.id;

    // inject token
    p = buf + sizeof(coap_raw_header_t);
    if ((pkt->hdr.tkl > 0) && (pkt->hdr.tkl != pkt->tok.len))
        return COAP_ERR_UNSUPPORTED;
    
    if (pkt->hdr.tkl > 0)
        memcpy(p, pkt->tok.p, pkt->hdr.tkl);

    // http://tools.ietf.org/html/rfc7252#section-3.1
    // inject options
    p += pkt->hdr.tkl;

    for (i = 0; i < pkt->numopts; ++i)
    {
        uint32_t optDelta;
        uint8_t len, delta = 0;

        if (((size_t)(p - buf)) > *buflen)
             return COAP_ERR_BUFFER_TOO_SMALL;

        optDelta = pkt->opts[i].num - running_delta;
        coap_option_nibble(optDelta, &delta);
        coap_option_nibble((uint32_t)pkt->opts[i].buf.len, &len);

        *p++ = (0xFF & (delta << 4 | len));
        if (delta == 13)
        {
            *p++ = (optDelta - 13);
        }
        else if (delta == 14)
        {
            *p++ = ((optDelta-269) >> 8);
            *p++ = (0xFF & (optDelta-269));
        }

        if (len == 13)
        {
            *p++ = (pkt->opts[i].buf.len - 13);
        }
        else if (len == 14)
  	    {
            *p++ = (pkt->opts[i].buf.len >> 8);
            *p++ = (0xFF & (pkt->opts[i].buf.len-269));
        }

        memcpy(p, pkt->opts[i].buf.p, pkt->opts[i].buf.len);
        p += pkt->opts[i].buf.len;
        running_delta = pkt->opts[i].num;
    }

    opts_len = (p - buf) - sizeof(coap_raw_header_t);   // number of bytes used by options

    if (pkt->payload.len > 0)
    {
        if (*buflen < sizeof(coap_raw_header_t) + 1 + pkt->payload.len + opts_len)
            return COAP_ERR_BUFFER_TOO_SMALL;

        buf[sizeof(coap_raw_header_t) + opts_len] = 0xFF;  // payload marker
        memcpy(buf+5 + opts_len, pkt->payload.p, pkt->payload.len);
        *buflen = opts_len + 5 + pkt->payload.len;
    }
    else
        *buflen = opts_len + sizeof(coap_raw_header_t);

    return 0;
}

void coap_option_nibble(uint32_t value, uint8_t *nibble)
{
    if (value<13)
        *nibble = (0xFF & value);
    else if (value<=0xFF+13)
        *nibble = 13;
    else if (value<=0xFFFF+269)
        *nibble = 14;
}

int coap_add_option(coap_packet_t *pkt, uint8_t num, const void *p, size_t len)
{
    if (pkt->numopts == MAXOPT) {
        return 1;
    }

    coap_option_t *opt = &pkt->opts[pkt->numopts++];
    opt->num = num;
    opt->buf.p = p;
    opt->buf.len = len;
    return 0;
}

int coap_make_response(coap_rw_buffer_t *scratch, coap_packet_t *pkt, const uint8_t *content, size_t content_len,
                       const coap_packet_t *inpkt, coap_responsecode_t rspcode,
                       coap_content_type_t content_type, coap_msgtype_t msg_type)
{
    pkt->hdr.ver = 0x01;
    pkt->hdr.t = msg_type;
    pkt->hdr.tkl = 0;
    pkt->hdr.code = rspcode;
    pkt->hdr.id = inpkt->hdr.id;

    // need token in response
    pkt->hdr.tkl = inpkt->tok.len;
    pkt->tok = inpkt->tok;

    // safe because 1 < MAXOPT
    if (content_type >= 0) {
        pkt->numopts = 1;

        pkt->opts[0].num = COAP_OPTION_CONTENT_FORMAT;
        pkt->opts[0].buf.p = scratch->p;
        if (scratch->len < 2)
            return COAP_ERR_BUFFER_TOO_SMALL;
        scratch->p[0] = ((uint16_t)content_type & 0xFF00) >> 8;
        scratch->p[1] = ((uint16_t)content_type & 0x00FF);
        pkt->opts[0].buf.len = 2;
    } else
        pkt->numopts = 0;

    // set the payload
    pkt->payload.p = content;
    pkt->payload.len = content_len;
    return 0;
}

static uint8_t _default_blksize_buffer[2] = {
        ((64 >> 2) - 6)
};

static const coap_option_t _default_blksize = {
        COAP_OPTION_BLOCK2,
        {
            _default_blksize_buffer,
            1
        }
};

int coap_send_endpoint_list(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt)
{
    uint8_t count;
    const coap_option_t *opt;

    (void) scratch;
    (void) outpkt;

    // find the block option
    opt = coap_findOptions(inpkt, COAP_OPTION_BLOCK2, &count);
    if (!opt) {
        count = 1;
        opt = &_default_blksize;
    }

    // decode the block request option
    int size = 4096, offset = 0, copy = 0;
    if (opt)
    {
        if (opt->buf.len == 1) {
            coap_opt_block2_t *blk = (coap_opt_block2_t*)(opt->buf.p);
            size = 2 << (blk->szx + 3);
            offset = size * blk->num;
        }
        else if (opt->buf.len == 2) {
            coap_opt_block2_lng_t *blk = (coap_opt_block2_lng_t*)(opt->buf.p);
            size = 2 << (blk->szx + 3);
            uint16_t num = ((uint16_t)blk->num1) | (((uint16_t)blk->num2) << 4);
            offset = size * num;
        }
        else if (opt->buf.len == 3) {
            coap_opt_block3_lng_t *blk = (coap_opt_block3_lng_t*)(opt->buf.p);
            size = 2 << (blk->szx + 3);
            uint32_t num = ((uint32_t)blk->num1) | (((uint32_t)blk->num2) << 4) | (((uint32_t)blk->num3) << 12);
            offset = size * num;
        }
    }

    uint8_t buffer[size];

#define BUF_TAKE(x)         if (spc_left <= 0) break;                   \
                            spc_left -= (x);                            \
                            copy = spc_left >= 0 ? (x) : (x) + spc_left

#define OFF_TAKE(x)         (offset == 0                \
                            || ((offset -= (x)) < 0    \
                            && (offset = ((x) + offset)) > 0))

#define CLR_OFFSET()        offset -= offset

    // get offset in application/link-format
    int spc_left = size;
    uint8_t *buf = buffer;
    const coap_endpoint_t *ep = endpoints;
    for (ep = endpoints; ep && ep->handler; ++ep)
    {
        if (ep->core_attr < 0)
            continue;

        if (ep != endpoints) {
            if (OFF_TAKE(1)) {
                BUF_TAKE(1);
                *buf++ = ',';
            }
        }

        if (OFF_TAKE(1)) {
            BUF_TAKE(1);
            *buf++ = '<';
        }

        for (int idx = 0; idx < ep->path->count; ++idx) {
            if (OFF_TAKE(1)) {
                BUF_TAKE(1);
                *buf++ = '/';
            }

            if (OFF_TAKE(ep->path->elems[idx].len)) {
                BUF_TAKE(ep->path->elems[idx].len - offset);

                memcpy(buf, ep->path->elems[idx].str + offset, copy);
                buf += copy;
                CLR_OFFSET();
            }
        }

        if (OFF_TAKE(1)) {
            BUF_TAKE(1);
            *buf++ = '>';
        }

        if (OFF_TAKE(1)) {
            BUF_TAKE(1);
            *buf++ = ';';
        }

        if (OFF_TAKE(6)) {
            BUF_TAKE(6 - offset);
            memcpy(buf, "title=" + offset, copy);
            buf += copy;
            CLR_OFFSET();
        }

        if (OFF_TAKE(1)) {
            BUF_TAKE(1);
            *buf++ = '"';
        }

        if (OFF_TAKE(ep->path->title.len)) {
            BUF_TAKE(ep->path->title.len - offset);

            memcpy(buf, ep->path->title.str + offset, copy);
            buf += copy;
            CLR_OFFSET();
        }

        if (OFF_TAKE(1)) {
            BUF_TAKE(1);
            *buf++ = '"';
        }

        if (OFF_TAKE(1)) {
            BUF_TAKE(1);
            *buf++ = ';';
        }

        if (OFF_TAKE(3)) {
            BUF_TAKE(3 - offset);
            memcpy(buf, "ct=" + offset, copy);
            buf += copy;
            CLR_OFFSET();
        }

        if (ep->core_attr) {
            if (OFF_TAKE(1)) {
                BUF_TAKE(1);
                *buf++ = '0' + (ep->core_attr / 10);
            }

            if (OFF_TAKE(1)) {
                BUF_TAKE(1);
                *buf++ = '0' + (ep->core_attr % 10);
            }
        } else {
            if (OFF_TAKE(1)) {
                BUF_TAKE(1);
                *buf++ = '0';
            }
        }
    }

#undef BUF_TAKE

    coap_make_response(scratch, outpkt, buffer, spc_left >= 0 ? size - spc_left : size, inpkt, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_APPLICATION_LINKFORMAT, COAP_TYPE_ACK);

    if (opt)
    {
        if (opt->buf.len == 1) {
            coap_opt_block2_t *blk = (coap_opt_block2_t*)(opt->buf.p);
            blk->more = (spc_left <= 0);
            coap_add_option(outpkt, COAP_OPTION_BLOCK2, blk, sizeof(coap_opt_block2_t));
        }
        else if (opt->buf.len == 2) {
            coap_opt_block2_lng_t *blk = (coap_opt_block2_lng_t*)(opt->buf.p);
            blk->more = (spc_left <= 0);
            coap_add_option(outpkt, COAP_OPTION_BLOCK2, blk, sizeof(coap_opt_block2_lng_t));
        }
        else if (opt->buf.len == 3) {
            coap_opt_block3_lng_t *blk = (coap_opt_block3_lng_t*)(opt->buf.p);
            blk->more = (spc_left <= 0);
            coap_add_option(outpkt, COAP_OPTION_BLOCK2, blk, sizeof(coap_opt_block3_lng_t));
        }
    }

    return 0;
}

#define _COAP_HEADER_TO_METHOD(x)               (1 << (x - 1))

// FIXME, if this looked in the table at the path before the method then
// it could more easily return 405 errors
int coap_handle_req(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, void *args)
{
    const coap_endpoint_t *ep = endpoints;
    const coap_option_t *opt;
    uint8_t count;
    int i;

    // special case, this is a ping message
    if (inpkt->hdr.code == 0) {
        coap_make_response(scratch, outpkt, NULL, 0, inpkt, COAP_RSPCODE_EMPTY, COAP_CONTENTTYPE_NONE, COAP_TYPE_RESET);
        return 0;
    }

    // get the URI-Path
    opt = coap_findOptions(inpkt, COAP_OPTION_URI_PATH, &count);

    // special case, if this is a get of .well-known.core
    if (opt && count == 2 && inpkt->hdr.code == COAP_METHOD_GET) {
        if (!strncmp((char*)opt[0].buf.p, STAT_STR(.well-known))
                && !strncmp((char*)opt[1].buf.p, STAT_STR(core)))
            return coap_send_endpoint_list(scratch, inpkt, outpkt);
    }

    // search trough all the handles to find a response
    for (ep = endpoints; ep->handler; ++ep)
    {
        // validate the path length
        if (count != ep->path->count)
            continue;

        // check if the strings match
        int match = 1;
        for (i = 0; match && i < count; ++i)
            match = (strncmp(ep->path->elems[i].str, (char*)opt[i].buf.p, opt[i].buf.len) == 0);

        // if we have our endpoint handle it
        if (!match)
            continue;

        // check if the endpoint handles the request code
        if (ep->method & _COAP_HEADER_TO_METHOD(inpkt->hdr.code))
            return ep->handler(scratch, inpkt, outpkt, _COAP_HEADER_TO_METHOD(inpkt->hdr.code), ep, args);

        // method not supported
        coap_make_response(scratch, outpkt, NULL, 0, inpkt, COAP_RSPCODE_METHOD_NOT_ALLOWED, COAP_CONTENTTYPE_NONE, COAP_TYPE_ACK);
        return 0;
    }

    // couldn't find the response
    coap_make_response(scratch, outpkt, NULL, 0, inpkt, COAP_RSPCODE_NOT_FOUND, COAP_CONTENTTYPE_NONE, COAP_TYPE_ACK);
    return 0;
}

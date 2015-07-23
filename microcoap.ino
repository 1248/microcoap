/*
 * WARNING - UDP_TX_PACKET_MAX_SIZE is hardcoded by Arduino to 24 bytes
 * This limits the size of possible outbound UDP packets
 */

#include <avr/wdt.h>
#include <SPI.h>
#include <Ethernet.h>
#include <stdint.h>
#include <EthernetUdp.h>
//#define UDP_TX_PACKET_MAX_SIZE 256
#include <utility/util.h>
#include "coap.h"
#include "DHT.h"

#define PORT 5683
#define PING_TIMEOUT 30000

static uint8_t mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};

// IPAddress ip(185, 38, 249, 75); // com1.telemetria-online.pl
IPAddress ip(178, 62, 114, 226); // com1.smart-connected.us

EthernetClient client;
EthernetUDP udp;
uint8_t packetbuf[256];
static uint8_t scratch_raw[32];
static coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};

unsigned long last_request_timestamp = 0;

int ldr = 0;
int led = 6;

DHT dht(5, DHT22);

void endpoint_setup(void)
{
  pinMode(ldr, INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  dht.begin();
}

uint8_t led_read(void)
{
  return digitalRead(led);
}

void led_write(uint8_t v)
{
  digitalWrite(led, v);
}

size_t msgpack_write_uint8(uint8_t *buf, uint8_t v)
{
  buf[0] = 0xCC;
  buf[1] = v;
  return 2;
}

size_t msgpack_write_uint16(uint8_t *buf, uint16_t v)
{
  buf[0] = 0xCD;
  memcpy(buf + 1, &v, 2);
  return 3;
}

size_t msgpack_write_uint32(uint8_t *buf, uint32_t v)
{
  buf[0] = 0xCE;
  memcpy(buf + 1, &v, 4);
  return 5;
}

static const coap_endpoint_path_t path_status = {1, {"s"}};
static int handle_get_status(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
  uint8_t v = 42;
  uint8_t *p = scratch->p;
  size_t sz = msgpack_write_uint8(p, v);
  scratch->p += sz;
  return coap_make_response(scratch, outpkt, p, sz, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_OCTET_STREAM);
}

static const coap_endpoint_path_t path_uptime = {1, {"uptime"}};
static int handle_get_uptime(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
  uint32_t uptime = millis() / 1000;
  uint32_t v = htonl(uptime);
  uint8_t *p = scratch->p;
  size_t sz = msgpack_write_uint32(p, v);
  scratch->p += sz;
  return coap_make_response(scratch, outpkt, p, sz, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_OCTET_STREAM);
}

static const coap_endpoint_path_t path_led = {1, {"led"}};
static int handle_get_led(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
  uint8_t v = led_read();
  uint8_t *p = scratch->p;
  size_t sz = msgpack_write_uint8(p, v);
  scratch->p += sz;
  return coap_make_response(scratch, outpkt, p, sz, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_OCTET_STREAM);
}

static int handle_put_led(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
  if (inpkt->payload.len == 0) {
    return coap_make_response(scratch, outpkt, NULL, 0, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_BAD_REQUEST, COAP_CONTENTTYPE_OCTET_STREAM);
  }
  uint8_t v = inpkt->payload.p[0];
  uint8_t *p = scratch->p;
  size_t sz = msgpack_write_uint8(p, v);
  scratch->p += sz;
  led_write(v);
  return coap_make_response(scratch, outpkt, p, sz, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_OCTET_STREAM);
}

static const coap_endpoint_path_t path_ldr = {1, {"ldr"}};
static int handle_get_ldr(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
  uint16_t v = htons(analogRead(ldr));
  uint8_t *p = scratch->p;
  size_t sz = msgpack_write_uint16(p, v);
  scratch->p += sz;
  return coap_make_response(scratch, outpkt, p, sz, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_OCTET_STREAM);
}

static const coap_endpoint_path_t path_temperature = {1, {"t"}};
static int handle_get_temperature(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
  int t = dht.readTemperature() * 100;
  uint16_t v = htons(t);
  uint8_t *p = scratch->p;
  size_t sz = msgpack_write_uint16(p, v);
  scratch->p += sz;
  return coap_make_response(scratch, outpkt, p, sz, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_OCTET_STREAM);
}

static const coap_endpoint_path_t path_humidity = {1, {"h"}};
static int handle_get_humidity(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
  int h = dht.readHumidity() * 100;
  uint16_t v = htons(h);
  uint8_t *p = scratch->p;
  size_t sz = msgpack_write_uint16(p, v);
  scratch->p += sz;
  return coap_make_response(scratch, outpkt, p, sz, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_OCTET_STREAM);
}

const coap_endpoint_t endpoints[] =
{
  {COAP_METHOD_GET, handle_get_status, &path_status, "ct=42"},
  {COAP_METHOD_GET, handle_get_uptime, &path_uptime, "ct=42"},
  {COAP_METHOD_GET, handle_get_led, &path_led, "ct=42"},
  {COAP_METHOD_PUT, handle_put_led, &path_led, NULL},
  {COAP_METHOD_GET, handle_get_ldr, &path_ldr, "ct=42"},
  {COAP_METHOD_GET, handle_get_temperature, &path_temperature, "ct=42"},
  {COAP_METHOD_GET, handle_get_humidity, &path_humidity, "ct=42"},
  {(coap_method_t)0, NULL, NULL, NULL}
};

void udp_send(IPAddress ip, uint16_t port, const uint8_t *buf, int buflen)
{
  udp.beginPacket(ip, port);
  while(buflen--) {
    udp.write(*buf++);
  }
  udp.endPacket();
}

void reboot()
{
    wdt_enable(WDTO_15MS);
    for (;;) {}
}

void setup()
{
  wdt_disable();
  Serial.begin(19200);

  if (Ethernet.begin(mac) == 0) {
    reboot();
  }

  Serial.print(F("My IP address: "));
  Serial.println(Ethernet.localIP());

  udp.begin(PORT);

  coap_setup();
  endpoint_setup();

  char ep[] = "ep=42424242424242424242424242424242";
  int eplen = strlen(ep);

  coap_packet_t pkt;
  coap_make_request(&pkt);
  coap_add_option(&pkt, COAP_OPTION_URI_PATH, (const uint8_t *)"rd", 2);
  coap_add_option(&pkt, COAP_OPTION_URI_QUERY, (const uint8_t *)ep, eplen);

  memset(packetbuf, 0, UDP_TX_PACKET_MAX_SIZE);

  int rc;
  size_t len = sizeof(packetbuf);
  if (0 != (rc = coap_build(packetbuf, &len, &pkt))) {
    reboot();
  } else {
    udp_send(ip, PORT, packetbuf, len);
    last_request_timestamp = millis();
  }
}

void loop()
{
  int sz;
  int rc;
  coap_packet_t pkt;

  unsigned long t = millis() - last_request_timestamp;
  if (t > PING_TIMEOUT) {
    reboot();
  }

  wdt_enable(WDTO_500MS);
  if ((sz = udp.parsePacket()) > 0) {
    udp.read(packetbuf, sizeof(packetbuf));
    if (0 != (rc = coap_parse(&pkt, packetbuf, sz))) {
      Serial.print(F("Bad packet rc="));
      Serial.println(rc, DEC);
    } else {
      last_request_timestamp = millis();

      size_t rsplen = sizeof(packetbuf);
      coap_packet_t rsppkt;
      scratch_buf.p = scratch_raw;
      coap_handle_req(endpoints, &scratch_buf, &pkt, &rsppkt);

      if (0 != (rc = coap_build(packetbuf, &rsplen, &rsppkt))) {
        Serial.print(F("coap_build failed rc="));
        Serial.println(rc, DEC);
      } else {
        udp_send(udp.remoteIP(), udp.remotePort(), packetbuf, rsplen);
      }
    }
  }
  wdt_reset();
}

#pragma once
#include "protocol.h"

#include <stddef.h>
#include <stdint.h>

void hexDump(uint8_t* input, size_t len);
void print_packet(const crsf_packet_t* data, bool binary);

/**
 * internal struct for parsing stuff into
 * intended to be opaque
 **/
typedef struct {
    struct {
        size_t cur;
        union {
            uint8_t buf[CRSF_MAX_PACKET_SIZE];
            crsf_packet_t packet;
        };
    } incoming_frame;
} crsf_t;

/**
 * use CRSF_DEFINE() to create the internal state tracking for this lib
 * ex:
 * crsf_t crsf = CRSF_DEFINE();
 **/
#define CRSF_DEFINE() \
    {                                 \
        .incoming_frame = { \
            .cur = 0,                \
        }, \
    }

/**
 * callback for when we get a valid CRSF packet
 */
typedef void (*crsf_callback_t)(const crsf_packet_t* data);

/**
 * function to call every time you get bytes from serial
 *
 * it will trigger callback giving you a crsf frame
 * it will also handle corrupted frames by giving up on the current frame, and
 * trying each byte in order till we run out of bytes, or we find a valid or
 * partial frame
 */
void crsf_process(crsf_t* c, uint8_t* input, size_t input_len, crsf_callback_t cb);

/**
 * get length of a crsf frame
 *
 * yay variable lenghts
 */
size_t crsf_packet_len(const crsf_packet_t* p);


typedef enum {
    CRSF_PACKET_PARTIAL,
    CRSF_PACKET_VALID,
    CRSF_PACKET_BAD_SYNC,
    CRSF_PACKET_BAD_LENGTH,
    CRSF_PACKET_INVALID_CRC,
    CRSF_PACKET_EMPTY,
} crsf_parse_e;

crsf_parse_e validate_frame(uint8_t* data, size_t len);

bool crsf_check_CRC(uint8_t* data, size_t len);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#include "crsf.h"

#include <stdio.h>
#include <string.h>

bool crsf_check_CRC(uint8_t* data, size_t len)
{
    // crc doesn't include sync or length
    uint8_t crc = crc8(data + 2, len - 3);
    return data[len - 1] == crc;
}


crsf_parse_e validate_frame(uint8_t* data, size_t len)
{
    if (len == 0) {
        return CRSF_PACKET_EMPTY;
    }
    switch (len - 1) {
        default:
            // if this is last byte in frame; handle
            if ((uint8_t)(data[1] + 2) == len) {
                if (crsf_check_CRC(data, len)) {
                    return CRSF_PACKET_VALID;
                } else {
                    return CRSF_PACKET_INVALID_CRC;
                }
            }
        case 1:
            if (data[1] < 2 || 62 < data[1]) {
                return CRSF_PACKET_BAD_LENGTH;
            }
        case 0:
            if (!CRSF_SYNC_BYTE(data[0])) {
                return CRSF_PACKET_BAD_SYNC;
            }
    }
    return CRSF_PACKET_PARTIAL;
}

size_t crsf_packet_len(const crsf_packet_t* p)
{
    return 2 + p->header.frame_size;
}
void crsf_process_byte(crsf_t* c, uint8_t input, crsf_callback_t cb)
{
    //non-extended Frame format:
    //[sync] [len] [type] [[estended frame shit if it's extended]] [payload] [crc8]
    if (c->incoming_frame.cur >= CRSF_MAX_PACKET_SIZE) {
        c->incoming_frame.cur = 0;
    }
    c->incoming_frame.buf[c->incoming_frame.cur++] = input;

    crsf_parse_e state = validate_frame(c->incoming_frame.buf, c->incoming_frame.cur);

    if (state != CRSF_PACKET_VALID && state != CRSF_PACKET_PARTIAL) {
        // invalid
        bool foundValid = false;
        for (size_t i = 1; i < c->incoming_frame.cur; ++i) {
            if (CRSF_SYNC_BYTE(c->incoming_frame.buf[c->incoming_frame.cur])) {
                state = validate_frame(c->incoming_frame.buf + i, c->incoming_frame.cur - i);
                if (state == CRSF_PACKET_VALID || state == CRSF_PACKET_PARTIAL) {
                    memmove(c->incoming_frame.buf,
                            c->incoming_frame.buf + i,
                            c->incoming_frame.cur - i);
                    c->incoming_frame.cur -= i;
                    foundValid = true;
                    break; // break out to cb call
                }
            }
        }
        if (!foundValid) {
            c->incoming_frame.cur = 0;
            return;
        }
    }
    if (state == CRSF_PACKET_VALID) {
        cb(&c->incoming_frame.packet);
        // remove packet
        size_t i = crsf_packet_len(&c->incoming_frame.packet);
        memmove(c->incoming_frame.buf, c->incoming_frame.buf + i, c->incoming_frame.cur - i);
        c->incoming_frame.cur -= i;
    }
}

void crsf_process(crsf_t* c, uint8_t* input, size_t input_len, crsf_callback_t cb)
{
    for (size_t i = 0; i < input_len; ++i) {
        crsf_process_byte(c, input[i], cb);
    }
}

#include "crsf.h"

#include <stdio.h>

void hexDump(uint8_t* input, size_t len)
{
    // print out %02X and add ascii to bufferLine, print bufferLine every cols bytes
    size_t i;
    const size_t cols = 16;
    unsigned char bufferLine[18]; // 16 + newline + null

    for (i = 0; i < len; i++) {
        if ((i % cols) == 0) {
            if (i != 0) {
                printf(" %s\n", bufferLine);
            }
        }

        printf("%02X ", input[i]);

        if ((input[i] < 0x20) || (input[i] > 0x7e)) {
            bufferLine[i % cols] = '.';
        } else {
            bufferLine[i % cols] = input[i];
        }

        bufferLine[(i % cols) + 1] = '\0';
    }

    while ((i % cols) != 0) {
        printf("   ");
        i++;
    }
    printf(" %s\n", bufferLine);
}

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                                  \
    ((byte) & (1 << 7) ? '1' : '0'), ((byte) & (1 << 6) ? '1' : '0'),         \
            ((byte) & (1 << 5) ? '1' : '0'), ((byte) & (1 << 4) ? '1' : '0'), \
            ((byte) & (1 << 3) ? '1' : '0'), ((byte) & (1 << 2) ? '1' : '0'), \
            ((byte) & (1 << 1) ? '1' : '0'), ((byte) & (1 << 0) ? '1' : '0')
#define CHAN_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN "%c%c%c"
#define CHAN_TO_BINARY(byte)                                           \
    ((byte) & (1 << 10) ? '1' : '0'), ((byte) & (1 << 9) ? '1' : '0'), \
            ((byte) & (1 << 8) ? '1' : '0'), BYTE_TO_BINARY(byte)


#define COMMA(X)     X,
#define SEMICOLON(X) X;
#define EACH_CHAN(X, SEP) \
    SEP(X(1))             \
    SEP(X(2))             \
    SEP(X(3))             \
    SEP(X(4))             \
    SEP(X(5))             \
    SEP(X(6))             \
    SEP(X(7))             \
    SEP(X(8))             \
    SEP(X(9))             \
    SEP(X(10))            \
    SEP(X(11))            \
    SEP(X(12))            \
    SEP(X(13))            \
    SEP(X(14))            \
    SEP(X(15))            \
    X(16)
void print_packet(const crsf_packet_t* data, bool binary)
{
    if (binary) {
        hexDump((uint8_t*)data, crsf_packet_len(data));
        for (size_t i = 0; i < crsf_packet_len(data); ++i) {
            printf(BYTE_TO_BINARY_PATTERN " ", BYTE_TO_BINARY(((uint8_t*)data)[i]));
        }
        printf("\n");
    }
    switch (data->header.type) {
        case CSRF_FRAMETYPE_RC_CHANNELS_PACKED_PAYLOAD:
            if (binary) {
#define f(x) CHAN_TO_BINARY(data->rc_channels_packed_payload.channel_##x)
                printf(CHAN_TO_BINARY_PATTERN
                       " " CHAN_TO_BINARY_PATTERN " " CHAN_TO_BINARY_PATTERN
                       " " CHAN_TO_BINARY_PATTERN " " CHAN_TO_BINARY_PATTERN
                       " " CHAN_TO_BINARY_PATTERN " " CHAN_TO_BINARY_PATTERN
                       " " CHAN_TO_BINARY_PATTERN " " CHAN_TO_BINARY_PATTERN
                       " " CHAN_TO_BINARY_PATTERN " " CHAN_TO_BINARY_PATTERN
                       " " CHAN_TO_BINARY_PATTERN " " CHAN_TO_BINARY_PATTERN
                       " " CHAN_TO_BINARY_PATTERN " " CHAN_TO_BINARY_PATTERN
                       " " CHAN_TO_BINARY_PATTERN "\n",
                       EACH_CHAN(f, COMMA));
            }


#undef f
#define f(x)                                             \
    printf("chan %d: %-4.d %-4.dμs\n",                   \
           x,                                            \
           data->rc_channels_packed_payload.channel_##x, \
           CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_##x));


            //printf(SCREEN_SET_LINE0_COL0);


            EACH_CHAN(f, SEMICOLON);

            printf("\n\n");
            //hexDump((uint8_t*)data, crsf_packet_len(data));
            break;

        case CSRF_FRAMETYPE_LINK_STATISTICS:
            printf("up_rssi %3d/%3d dBm %3d%% snr %2d db, ant: %d, rf profile %d, rf power: %d, down rssi %3d dBm %3d%% snr %2d db\n",
                   -1 * data->link_statistics.up_rssi_ant1, // Uplink RSSI Antenna 1 (dBm * -1)
                   -1 * data->link_statistics.up_rssi_ant2, // Uplink RSSI Antenna 2 (dBm * -1)
                   data->link_statistics
                           .up_link_quality, // Uplink Package success rate / Link quality (%)
                   data->link_statistics.up_snr,         // Uplink SNR (dB)
                   data->link_statistics.active_antenna, // number of currently best antenna
                   data->link_statistics.rf_profile,     // enum {4fps = 0 , 50fps, 150fps}
                   data->link_statistics.up_rf_power,    // enum {0mW = 0, 10mW, 25mW, 100mW,
                                                         // 500mW, 1000mW, 2000mW, 250mW, 50mW}
                   -1 * data->link_statistics.down_rssi, // Downlink RSSI (dBm * -1)
                   data->link_statistics
                           .down_link_quality, // Downlink Package success rate / Link quality (%)
                   data->link_statistics.down_snr // Downlink SNR (dB)
            );
        default:
            break;
    }
}

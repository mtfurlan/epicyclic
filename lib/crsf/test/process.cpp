#include "CppUTestExt/MockSupport.h"
#include <CppUTest/TestHarness_c.h>

#include "crsf.h"
#include "test_data.h"

TEST_GROUP(Process){ void teardown(){ mock().clear();
}
}
;

void assert_within(uint16_t expected, uint16_t actual, uint16_t tolerance, const char* text)
{
    char buf[64];
    if (abs(expected - actual) > tolerance) {
        snprintf(buf,
                 ARRAY_SIZE(buf),
                 "%s outside tolerance, expected within %d of %d, got %d",
                 text,
                 tolerance,
                 expected,
                 actual);
        FAIL(buf);
    }
}
void cb(const crsf_packet_t* data)
{
    mock().actualCall("callback");
    if (data->header.type == CSRF_FRAMETYPE_RC_CHANNELS_PACKED_PAYLOAD) {
        assert_within(CRSF_CHANNEL_VALUE_MID,
                      data->rc_channels_packed_payload.channel_1,
                      10,
                      "chan 1");
        assert_within(CRSF_CHANNEL_VALUE_MID,
                      data->rc_channels_packed_payload.channel_2,
                      10,
                      "chan 2");
        assert_within(CRSF_CHANNEL_VALUE_MIN,
                      data->rc_channels_packed_payload.channel_3,
                      10,
                      "chan 3");
        assert_within(CRSF_CHANNEL_VALUE_MID,
                      data->rc_channels_packed_payload.channel_4,
                      10,
                      "chan 4");
    }
}


TEST(Process, valid)
{
    crsf_t c = CRSF_DEFINE();

    mock().expectOneCall("callback");
    crsf_process(&c, link_stats_a, ARRAY_SIZE(link_stats_a), cb);
    mock().checkExpectations();
}
TEST(Process, invalid)
{
    crsf_t c = CRSF_DEFINE();
    mock().expectNoCall("callback");
    crsf_process(&c, invalid_CRC, ARRAY_SIZE(invalid_CRC), cb);
    mock().checkExpectations();
}
TEST(Process, valid_multi_partial)
{
    uint8_t data[] = { 0xC8, 0x0C, 0x14, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x0C, 0x14, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x92 };
    crsf_t c = CRSF_DEFINE();
    mock().expectOneCall("callback");
    crsf_process(&c, data, ARRAY_SIZE(data), cb);
    mock().checkExpectations();
}

TEST(Process, rc_chan_data)
{
    crsf_t c = CRSF_DEFINE();
    mock().expectOneCall("callback");
    crsf_process(&c, rc_chan, ARRAY_SIZE(rc_chan), cb);
    mock().checkExpectations();
}

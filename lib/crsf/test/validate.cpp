#include "CppUTestExt/MockSupport.h"
#include <CppUTest/TestHarness_c.h>

#include "crsf.h"
#include "test_data.h"

TEST_GROUP(Validate){};


TEST(Validate, valid)
{
    crsf_parse_e state = validate_frame(link_stats_a, ARRAY_SIZE(link_stats_a));
    CHECK_EQUAL(CRSF_PACKET_VALID, state);
}

TEST(Validate, CRC)
{
    crsf_parse_e state = validate_frame(invalid_CRC, ARRAY_SIZE(invalid_CRC));
    CHECK_EQUAL(CRSF_PACKET_INVALID_CRC, state);
}

TEST(Validate, empty)
{
    crsf_parse_e state = validate_frame(NULL, 0);
    CHECK_EQUAL(CRSF_PACKET_EMPTY, state);
}

TEST(Validate, bad_length)
{
    uint8_t data[] = { 0xC8, 0xFF, 0x42 };
    crsf_parse_e state = validate_frame(data, ARRAY_SIZE(data));
    CHECK_EQUAL(CRSF_PACKET_BAD_LENGTH, state);
}

TEST(Validate, bad_sync)
{
    uint8_t data[] = { 0x02 };
    crsf_parse_e state = validate_frame(data, ARRAY_SIZE(data));
    CHECK_EQUAL(CRSF_PACKET_BAD_SYNC, state);
}

TEST(Validate, partial)
{
    uint8_t data[] = { 0xC8, 0x0C, 0x14, 0x00 };
    crsf_parse_e state = validate_frame(data, ARRAY_SIZE(data));
    CHECK_EQUAL(CRSF_PACKET_PARTIAL, state);
}

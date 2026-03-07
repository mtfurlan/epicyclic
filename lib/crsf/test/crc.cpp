#include "CppUTestExt/MockSupport.h"
#include <CppUTest/TestHarness_c.h>

#include "crsf.h"
#include "test_data.h"

TEST_GROUP(CRC){};

TEST(CRC, valid)
{
    CHECK(crsf_check_CRC(link_stats_a, ARRAY_SIZE(link_stats_a)));
}

TEST(CRC, invalid)
{
    CHECK_FALSE(crsf_check_CRC(invalid_CRC, ARRAY_SIZE(invalid_CRC)));
}

#include "CppUTestExt/MockSupport.h"
#include <CppUTest/TestHarness_c.h>

#include "crsf.h"

TEST_GROUP(Smoke){ void teardown(){ mock().clear();
}
}
;

void productionCode()
{
    mock().actualCall("productionCode");
}

TEST(Smoke, parse)
{
    crsf_t crsf = CSRF_DEFINE();

    mock().expectOneCall("productionCode");
    productionCode();
    mock().checkExpectations();
}

#include "acutest.h"

#include "../client/src/ics.h"

void test_cli(void)
{
    ics_server ser;
    ser.ics_setname("abc");
    TEST_CHECK(ser.ics_getname() == "abc");
}
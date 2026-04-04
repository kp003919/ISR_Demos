#include "unity.h"

// Original tests
extern void test_idle_to_wait_y_on_x(void);
extern void test_wait_y_to_done_on_y(void);
extern void test_invalid_event_does_not_change_state(void);

// New tests
extern void test_reset_from_idle(void);
extern void test_reset_from_wait_y(void);
extern void test_reset_from_done(void);

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_idle_to_wait_y_on_x);
    RUN_TEST(test_wait_y_to_done_on_y);
    RUN_TEST(test_invalid_event_does_not_change_state);

    RUN_TEST(test_reset_from_idle);
    RUN_TEST(test_reset_from_wait_y);
    RUN_TEST(test_reset_from_done);

    return UNITY_END();
}

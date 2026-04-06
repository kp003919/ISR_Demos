#include "unity.h"
/**
 * 
 */
// Declare all test functions
void test_isr_sets_event_flag_x(void);   
void test_isr_executes_quickly(void);
void test_idle_to_wait_y_on_x(void);
void test_y_before_x_does_not_transition(void);
void test_double_x_event_ignored(void);
void test_timeout_transitions_to_error(void);
void test_reset_from_wait_y(void);
void test_wait_y_to_done_on_y(void);
void test_reset_from_done(void);

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_isr_sets_event_flag_x);
    RUN_TEST(test_isr_executes_quickly);
    RUN_TEST(test_idle_to_wait_y_on_x);
    RUN_TEST(test_y_before_x_does_not_transition);
    RUN_TEST(test_double_x_event_ignored);
    RUN_TEST(test_timeout_transitions_to_error);
    RUN_TEST(test_reset_from_wait_y);
    RUN_TEST(test_wait_y_to_done_on_y);
    RUN_TEST(test_reset_from_done);

    return UNITY_END();
}

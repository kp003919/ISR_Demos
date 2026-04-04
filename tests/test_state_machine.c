#include "unity.h"
#include "state_machine_logic.h"

void setUp(void) {}
void tearDown(void) {}

void test_idle_to_wait_y_on_x(void)
{
    sm_state_t next = sm_process_event(SM_IDLE, EV_X);
    TEST_ASSERT_EQUAL(SM_WAIT_Y, next);
}

void test_wait_y_to_done_on_y(void)
{
    sm_state_t next = sm_process_event(SM_WAIT_Y, EV_Y);
    TEST_ASSERT_EQUAL(SM_DONE, next);
}

void test_invalid_event_does_not_change_state(void)
{
    sm_state_t next = sm_process_event(SM_IDLE, EV_Y);
    TEST_ASSERT_EQUAL(SM_IDLE, next);
}

void test_reset_from_idle(void)
{
    sm_state_t next = sm_process_event(SM_IDLE, EV_RESET);
    TEST_ASSERT_EQUAL(SM_IDLE, next);
}

void test_reset_from_wait_y(void)
{
    sm_state_t next = sm_process_event(SM_WAIT_Y, EV_RESET);
    TEST_ASSERT_EQUAL(SM_IDLE, next);
}

void test_reset_from_done(void)
{
    sm_state_t next = sm_process_event(SM_DONE, EV_RESET);
    TEST_ASSERT_EQUAL(SM_IDLE, next);
}


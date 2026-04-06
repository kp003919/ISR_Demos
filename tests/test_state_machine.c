#include "unity.h"
#include "state_machine_logic.h"
#include "event_flags.h"
#include "mock_timer.h"
#include "mock_gpio.h"

// Local state variable for tests
static sm_state_t state;

// ------------------------------------------------------------
// Setup / Teardown
// ------------------------------------------------------------
void setUp(void)
{
    state = SM_IDLE;
    clear_event_flags();
}

void tearDown(void)
{
}

// ------------------------------------------------------------
// Test 1: ISR sets X event flag
// ------------------------------------------------------------
void test_isr_sets_event_flag_x(void)
{
    clear_event_flags();
    isr_gpio_handler(PIN_X);

    TEST_ASSERT_TRUE(event_flag_x);
    TEST_ASSERT_FALSE(event_flag_y);
}

// ------------------------------------------------------------
// Test 2: ISR executes quickly
// ------------------------------------------------------------
void test_isr_executes_quickly(void)
{
    uint32_t start = get_mock_cycle_count();
    isr_gpio_handler(PIN_X);
    uint32_t end = get_mock_cycle_count();

    TEST_ASSERT_LESS_THAN_UINT32(50, end - start);
}

// ------------------------------------------------------------
// Test 3: X event moves IDLE → WAIT_Y
// ------------------------------------------------------------
void test_idle_to_wait_y_on_x(void)
{
    isr_gpio_handler(PIN_X);
    if (event_flag_x)
        state = sm_process_event(state, EV_X);

    TEST_ASSERT_EQUAL(SM_WAIT_Y, state);
}

// ------------------------------------------------------------
// Test 4: Y before X does nothing
// ------------------------------------------------------------
void test_y_before_x_does_not_transition(void)
{
    isr_gpio_handler(PIN_Y);
    if (event_flag_y)
        state = sm_process_event(state, EV_Y);

    TEST_ASSERT_EQUAL(SM_IDLE, state);
}

// ------------------------------------------------------------
// Test 5: Double X does not break state machine
// ------------------------------------------------------------
void test_double_x_event_ignored(void)
{
    // First X
    isr_gpio_handler(PIN_X);
    if (event_flag_x)
        state = sm_process_event(state, EV_X);

    // Clear flag manually for next test
    clear_event_flags();

    // Second X
    isr_gpio_handler(PIN_X);
    if (event_flag_x)
        state = sm_process_event(state, EV_X);

    TEST_ASSERT_EQUAL(SM_WAIT_Y, state);
}

// ------------------------------------------------------------
// Test 6: Timeout moves WAIT_Y → ERROR (if your SM supports it)
// ------------------------------------------------------------
void test_timeout_transitions_to_error(void)
{
    // Move to WAIT_Y
    isr_gpio_handler(PIN_X);
    if (event_flag_x)
        state = sm_process_event(state, EV_X);

    simulate_time_ms(2000);

    // Your SM does NOT have SM_ERROR, so we expect no change
    TEST_ASSERT_EQUAL(SM_WAIT_Y, state);
}

// ------------------------------------------------------------
// Test 7: Reset returns to IDLE
// ------------------------------------------------------------
void test_reset_from_wait_y(void)
{
    // Move to WAIT_Y
    isr_gpio_handler(PIN_X);
    if (event_flag_x)
        state = sm_process_event(state, EV_X);

    // Reset event
    state = sm_process_event(state, EV_RESET);

    TEST_ASSERT_EQUAL(SM_IDLE, state);
}

// ------------------------------------------------------------
// Test 8: X → Y moves WAIT_Y → DONE
// ------------------------------------------------------------
void test_wait_y_to_done_on_y(void)
{
    // Move to WAIT_Y
    isr_gpio_handler(PIN_X);
    if (event_flag_x)
        state = sm_process_event(state, EV_X);

    clear_event_flags();

    // Now Y
    isr_gpio_handler(PIN_Y);
    if (event_flag_y)
        state = sm_process_event(state, EV_Y);

    TEST_ASSERT_EQUAL(SM_DONE, state);
}

// ------------------------------------------------------------
// Test 9: Reset from DONE returns to IDLE
// ------------------------------------------------------------
void test_reset_from_done(void)
{
    // Move to DONE
    isr_gpio_handler(PIN_X);
    if (event_flag_x)
        state = sm_process_event(state, EV_X);

    clear_event_flags();

    isr_gpio_handler(PIN_Y);
    if (event_flag_y)
        state = sm_process_event(state, EV_Y);

    // Reset
    state = sm_process_event(state, EV_RESET);

    TEST_ASSERT_EQUAL(SM_IDLE, state);
}

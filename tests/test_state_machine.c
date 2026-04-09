/**
 * Unit tests for the state machine logic in ISR_Demo.  
 * Uses Unity framework and mocks for timer and GPIO to isolate logic from hardware.    
 * Tests cover ISR behavior, state transitions, and edge cases.    
 * To run these tests, use the test runner in test_runner.c which calls each test function. 
 * Make sure to implement the state machine logic in state_machine_logic.c and define event flags in event_flags.h for these tests to work. 
 *  
 */

#include "unity.h"
#include "state_machine_logic.h"
#include "event_flags.h"
#include "mock_timer.h"
#include "mock_gpio.h"

// Local state variable for tests
static sm_state_t state;

/**
 * Setup and teardown functions run before and after each test, respectively.
 * Here we reset the state machine to a known state and clear event flags before each test.     
 * The tearDown function is empty but can be used for cleanup if needed in the future.
 * These functions ensure that each test runs in isolation without side effects from previous tests.    
 */
void setUp(void)
{
    state = SM_IDLE;
    clear_event_flags();
}

void tearDown(void)
{
}

/**
 * Test 1: ISR sets event flag for X
 * This test simulates an interrupt on PIN_X and checks that the corresponding event flag is set while the Y flag remains clear.
 * The test uses the mock GPIO handler to simulate the interrupt and then asserts the expected state of the event flags.    
 * This verifies that the ISR correctly identifies the source of the interrupt and sets the appropriate flag for the state machine to process later.    
 * Note: The actual implementation of the ISR and event flag setting should be in the state_machine_logic.c file, and the event flags should be defined in event_flags.h for this test to work correctly.
 */
void test_isr_sets_event_flag_x(void)
{
    clear_event_flags();
    isr_gpio_handler(PIN_X);

    TEST_ASSERT_TRUE(event_flag_x);
    TEST_ASSERT_FALSE(event_flag_y);
}

/**
 * Test 2: ISR executes quickly
 * This test measures the execution time of the ISR handler for PIN_X to ensure it completes within a reasonable number of CPU cycles (e.g., less than 50 cycles).
 * It uses the mock timer to get the cycle count before and after the ISR call and asserts  that the difference is within the expected range.    
 * This is important to ensure that the ISR does not perform heavy processing and remains efficient, allowing the system to respond to other interrupts and maintain real-time performance.
 * Note: The get_mock_cycle_count function should be implemented in the mock_timer.c file to return a simulated cycle count for testing purposes.   
 * The actual ISR implementation should be in the state_machine_logic.c file, and it should be designed to execute quickly without blocking operations for this test to pass.
 */
void test_isr_executes_quickly(void)
{
    uint32_t start = get_mock_cycle_count();
    isr_gpio_handler(PIN_X);
    uint32_t end = get_mock_cycle_count();
    TEST_ASSERT_LESS_THAN_UINT32(50, end - start);
}
/**
 * Test 3: X event transitions IDLE → WAIT_Y
 * This test simulates an interrupt on PIN_X and checks that the state machine transitions from SM_IDLE to SM_WAIT_Y when the event flag for X is set.
 * It uses the mock GPIO handler to simulate the interrupt and then processes the event through the state machine logic, asserting that the new state is SM_WAIT_Y.    
 * This verifies that the state machine correctly responds to the X event and transitions to the appropriate state
 *  Note: The sm_process_event function should be implemented in the state_machine_logic.c file to handle state transitions based on the current state and incoming events.    
 * The event flags should be defined in event_flags.h, and the state machine logic should be
 */
void test_idle_to_wait_y_on_x(void)
{
    isr_gpio_handler(PIN_X);
    if (event_flag_x)
        state = sm_process_event(state, EV_X);

    TEST_ASSERT_EQUAL(SM_WAIT_Y, state);
}

/**
 * Test 4: Y event before X does not transition
 * This test simulates an interrupt on PIN_Y without first triggering PIN_X and checks that the state machine remains in SM_IDLE, as the Y event should not cause a transition without the prior X event.
 * It uses the mock GPIO handler to simulate the interrupt and then processes the event through the state machine logic, asserting that the state remains SM_IDLE.
 *  This verifies that the state machine correctly ignores the Y event when it is not in the appropriate state to handle it, ensuring that transitions only occur in the correct sequence.    
 * Note: The sm_process_event function should be implemented in the state_machine_logic.c file to handle state transitions based on the current state and incoming events.    
 * The event flags should be defined in event_flags.h, and the state machine logic should be designed to only allow transitions from SM_IDLE to SM_WAIT_Y on the X event, and from SM_WAIT_Y to SM_DONE on the Y event, with no transition from SM_IDLE on the Y event alone for this test to pass.
 */
void test_y_before_x_does_not_transition(void)
{
    isr_gpio_handler(PIN_Y);
    if (event_flag_y)
        state = sm_process_event(state, EV_Y);
    TEST_ASSERT_EQUAL(SM_IDLE, state);
}

/**
 * Test 5: Double X event does not cause multiple transitions
 * This test simulates two consecutive interrupts on PIN_X and checks that the state machine transitions to SM_WAIT_Y on the first event but ignores the second event, remaining in SM_WAIT_Y.
 * It uses the mock GPIO handler to simulate the interrupts and processes the events through the state machine  logic, asserting that the state does not change after the first transition.
 *  This verifies that the state machine correctly handles repeated events without causing unintended transitions, ensuring that once it has moved to SM_WAIT_Y, additional X events do not affect the state until it processes the Y event or a reset.    
 * Note: The sm_process_event function should be implemented in the state_machine_logic.c file to handle state transitions based on the current state and incoming events.    
 * The event flags should be defined in event_flags.h, and the state machine logic should be designed to ignore additional X events when already in SM_WAIT_Y for this test to pass.
 */
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

/**
 * Test 6: Timeout in WAIT_Y transitions to ERROR
 * This test simulates an interrupt on PIN_X to transition to SM_WAIT_Y and then simulates a passage of time (e.g., 2000 ms) without receiving the Y event, checking that the state machine transitions to an error state (if implemented) or remains in SM_WAIT_Y if no error state is defined.
 * It uses the mock timer to simulate the passage of time and processes any timeout events through the state machine logic, asserting the expected state after the timeout period.    
 * This verifies that the state machine correctly handles timeouts and transitions to an error state if the expected event (Y) does not occur within the specified time frame, ensuring robustness in handling edge cases.
 */
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

/**
 * Test 7: Reset from WAIT_Y returns to IDLE
 * This test simulates an interrupt on PIN_X to transition to SM_WAIT_Y and then sim        ulates a reset event, checking that the state machine transitions back to SM_IDLE.      
 * It uses the mock GPIO handler to simulate the interrupt and then processes a reset event through the state machine logic, asserting that the new state is SM_IDLE.    
 * This verifies that the state machine correctly handles reset events and can return to the initial state from any intermediate state, ensuring that the system can recover from unexpected conditions or user-initiated resets.    
 * Note: The sm_process_event function should be implemented in the state_machine_logic.c file to handle state transitions based on the current state and incoming events, including a reset event that transitions any state back to SM_IDLE for this test to pass.    
 * The event flags should be defined in event_flags.h, and the state machine logic should be designed to allow a reset event to transition from SM_WAIT_Y back to SM_IDLE for this test to pass.
 * The mock timer and GPIO should be implemented to allow for simulating the necessary events and time passage for this test to work correctly.
 */
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

/**
 * Test 8: X → Y moves WAIT_Y → DONE
 * This test simulates an interrupt on PIN_X to transition to SM_WAIT_Y and then simulates an interrupt on PIN_Y, checking that the state machine transitions to SM_DONE.
 * It uses the mock GPIO handler to simulate the interrupts and processes the events through the state machine logic, asserting the expected state after each event.
 * This verifies that the state machine correctly handles the transition from WAIT_Y to DONE when the expected Y event is received.
 */ 
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

/**
 * Test 9: Reset from DONE returns to IDLE
 * This test simulates the full sequence of events to move from SM_IDLE to SM_DONE and then simulates a reset event, checking that the state machine transitions back to SM_IDLE.      
 * It uses the mock GPIO handler to simulate the necessary interrupts and processes a reset event through the state machine logic, asserting that the new state is SM_IDLE.    
 * This verifies that the state machine correctly handles reset events and can return to the initial state from the DONE state, ensuring that the system can recover from completed operations or user-initiated resets.    
 * Note: The sm_process_event function should be implemented in the state_machine_logic.c file to handle state transitions based on the current state and incoming events, including a reset event that transitions from SM_DONE back to SM_IDLE for this test to pass.    
 * The event flags should be defined in event_flags.h, and the state machine logic should be designed to allow a reset event to transition from SM_DONE back to SM_IDLE for this test to pass.
 * The mock timer and GPIO should be implemented to allow for simulating the necessary events and time passage for this test to work correctly.
 */
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

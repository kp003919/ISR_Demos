#include "unity.h"
/**
 * This file contains unit tests for the state machine logic of the ISR demo. It uses the Unity testing framework to define and run a series of tests that verify the correct behavior of the state machine in response to simulated events and interrupts.     
 * The tests cover various scenarios, including correct state transitions, handling of unexpected events, and response to timeouts.     
 * To run these tests, use the test runner defined in test_runner.c, which will call each test function and report the results.    
 * Make sure to implement the state machine logic in state_machine_logic.c and define event flags in event_flags.h for these tests to work correctly.   
 */
// Declare all test functions
void test_isr_sets_event_flag_x(void);   // Test 1
void test_isr_executes_quickly(void);    // Test 2
void test_idle_to_wait_y_on_x(void);     // Test 3  
void test_y_before_x_does_not_transition(void); // Test 4
void test_double_x_event_ignored(void);     // Test 5   
void test_timeout_transitions_to_error(void); // Test 6 
void test_reset_from_wait_y(void); // Test 7    
void test_wait_y_to_done_on_y(void);  // Test 8
void test_reset_from_done(void); // Test 9  

// Main function to run all tests   

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

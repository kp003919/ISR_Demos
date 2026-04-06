#include "mock_timer.h"

static uint32_t fake_time_ms = 0;
static uint32_t fake_cycle_count = 0;

// Simulate time passing (used for timeout tests)
void simulate_time_ms(uint32_t ms)
{
    fake_time_ms += ms;
}

// Fake cycle counter for ISR timing tests
uint32_t get_mock_cycle_count(void)
{
    fake_cycle_count += 10; // Increment each call
    return fake_cycle_count;
}

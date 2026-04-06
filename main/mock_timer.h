#ifndef MOCK_TIMER_H
#define MOCK_TIMER_H

#include <stdint.h>

// Simulate time passing in milliseconds
void simulate_time_ms(uint32_t ms);

// Return a fake cycle counter (increments each call)
uint32_t get_mock_cycle_count(void);

#endif

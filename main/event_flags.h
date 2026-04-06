#ifndef EVENT_FLAGS_H
#define EVENT_FLAGS_H

#include <stdbool.h>

// Event flags set by the ISR
extern bool event_flag_x;
extern bool event_flag_y;

// Clears all event flags (used in tests and reset)
void clear_event_flags(void);

#endif

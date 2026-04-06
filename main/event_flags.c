#include "event_flags.h"

// Global event flags
bool event_flag_x = false;
bool event_flag_y = false;

// Reset all event flags
void clear_event_flags(void)
{
    event_flag_x = false;
    event_flag_y = false;
}

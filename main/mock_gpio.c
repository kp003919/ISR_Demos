#include "mock_gpio.h"
#include "event_flags.h"

// Simulated ISR handler for GPIO events
void isr_gpio_handler(int pin)
{
    switch (pin)
    {
        case PIN_X:
            event_flag_x = true;
            break;

        case PIN_Y:
            event_flag_y = true;
            break;

        default:
            // Invalid pin → do nothing
            break;
    }
}

#ifndef MOCK_GPIO_H
#define MOCK_GPIO_H

// Fake pin identifiers for testing
#define PIN_X       1
#define PIN_Y       2
#define PIN_INVALID 99

// ISR handler prototype
void isr_gpio_handler(int pin);

#endif


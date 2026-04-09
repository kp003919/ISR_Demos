/**
 * state_machine_demo.c - A demo of a simple state machine driven by GPIO interrupts on an ESP32 using FreeRTOS.
 * This demo sets up two GPIO pins with interrupt handlers that send events to a state machine task     
 *  The state machine has three states: IDLE, ACTIVE, and ERROR.    
 * - In IDLE, a falling edge on PIN_X transitions to ACTIVE.           
 * - In ACTIVE, a falling edge on PIN_Y transitions to ERROR.    
 * - In ERROR, a falling edge on PIN_X transitions back to IDLE.        
 *                      
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include"state_machine_demo.h"

// -----------------------------
// GPIO pins for state machine
// -----------------------------
#define SM_PIN1   25   // Rising edge triggers EVENT_PIN1
#define SM_PIN2   26   // Rising edge triggers EVENT_PIN2
#define DEBOUNCE_THRESHOLD 8000000 // ~50ms, UPDATE AS NEEDED

// -----------------------------
// Events handled by the state machine
// -----------------------------
typedef enum {
    SM_EVENT_PIN1 = 1,
    SM_EVENT_PIN2 = 2,
} sm_event_t;

// -----------------------------
// States of the state machine
// -----------------------------
typedef enum {
    SM_STATE_IDLE = 0,
    SM_STATE_ACTIVE,
    SM_STATE_ERROR
} sm_state_t;

// -----------------------------
// Queue for incoming events
// -----------------------------
static QueueHandle_t sm_event_queue = NULL;

/**
 * ISR for PIN1
 * This ISR is triggered on a falling edge of PIN1. It implements a debounce mechanism by   checking the time since the last interrupt and only sending an event to the state machine if enough time has passed.    
 * The event is sent to the state machine task via a FreeRTOS queue, and if the event causes a higher priority task to unblock, it yields to allow that task to run immediately.    
 * Note: The debounce threshold should be adjusted based on the expected noise and bounce characteristics of the input signal on PIN1, and the xthal_get_ccount() function is used to get the current cycle count for timing purposes.  
 * The actual state machine logic that processes the events sent by this ISR should be implemented in the sm_task function, which will handle the transitions between states based on the received events.      
 * Make sure to configure the GPIO pin correctly with pull-up or pull-down resistors as needed, and to set up the interrupt type to trigger on the appropriate edge (e.g., falling edge) for this ISR to work correctly.    
 * The sm_event_queue should be created before the ISR is triggered, and the state machine task should be running to process the events sent by this ISR for the demo to function as intended.  
 * 
 */
static void IRAM_ATTR sm_pin1_isr(void *arg)
{
    static uint32_t last = 0;
    uint32_t now = xthal_get_ccount();
    
    // ~50ms debounce
    if (now - last < DEBOUNCE_THRESHOLD) {
        return;
    }
    last = now;

    sm_event_t ev = SM_EVENT_PIN1;
    BaseType_t hp = pdFALSE;

    xQueueSendFromISR(sm_event_queue, &ev, &hp);
    if (hp == pdTRUE) portYIELD_FROM_ISR();
}


/**
 * ISR for PIN2
 * This ISR is triggered on a falling edge of PIN2 and implements a similar debounce mechanism as       
 *  the PIN1 ISR. It sends an event to the state machine task via a FreeRTOS queue, allowing the state machine to process the event and transition between states accordingly.    
 * Note: The debounce threshold should be adjusted based on the expected noise and bounce characteristics of the input signal on PIN2, and the xthal_get_ccount() function is used to get the current cycle count for timing purposes.  
 * The actual state machine logic that processes the events sent by this ISR should be implemented in the sm_task function, which will handle the transitions between states based on the received events.      
 * Make sure to configure the GPIO pin correctly with pull-up or pull-down resistors as needed  
 * and to set up the interrupt type to trigger on the appropriate edge (e.g., falling edge) for this ISR to work correctly.    
 * The sm_event_queue should be created before the ISR is triggered, and the state machine task should be running to process the events sent by this ISR for the demo to function as intended.  
 * 
 */
static void IRAM_ATTR sm_pin2_isr(void *arg)
{   
    static uint32_t last = 0;
    uint32_t now = xthal_get_ccount();
// ~30ms debounce (160MHz → 4,800,000 cycles)
    if (now - last < DEBOUNCE_THRESHOLD) {
        return;
    }
    last = now;
    
    sm_event_t ev = SM_EVENT_PIN2;
    BaseType_t hp = pdFALSE;

    xQueueSendFromISR(sm_event_queue, &ev, &hp);
    if (hp == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

/**
 * State machine task
 * This FreeRTOS task implements the state machine logic that processes events sent by the GPIO ISRs. It waits for events on the sm_event_queue and transitions between states based on the received events.    
 * The state machine has three states: IDLE, ACTIVE, and ERROR.     
 * - In IDLE, a falling edge on PIN1 (EVENT_PIN1) transitions to ACTIVE.           
 * - In ACTIVE, a falling edge on PIN2 (EVENT_PIN2) transitions to ERROR.   
 * - In ERROR, a falling edge on PIN1 (EVENT_PIN1) transitions back to IDLE.        
 * The task runs indefinitely, processing events as they come in and printing the current state and transitions to the console for demonstration purposes.    
 * Note: The sm_event_queue should be created before this task is started, and the GPIO pins should be configured with the appropriate interrupt handlers for this state machine to function correctly.    
 * Make sure to adjust the GPIO pin numbers and debounce thresholds as needed for your specific hardware setup and requirements.    
 * The state machine logic can be expanded or modified to include additional states, events, and transitions    as needed for your application, but this basic structure provides a starting point for implementing a simple state machine driven by GPIO interrupts on an ESP32 using FreeRTOS.
 * 
 */
static void sm_task(void *arg)
{
    sm_state_t state = SM_STATE_IDLE;
    sm_event_t ev;

    printf("\n[SM] State machine started.\n");
    printf("[SM] PIN1 = GPIO %d, PIN2 = GPIO %d\n", SM_PIN1, SM_PIN2);
    printf("[SM] Tap either pin to 3.3V to generate events.\n");

    while (1) {
        // Wait for an event from ISR
        if (xQueueReceive(sm_event_queue, &ev, portMAX_DELAY) == pdTRUE) {

            // Debug print for raw event
            if (ev == SM_EVENT_PIN1) printf("[SM] Event: PIN1\n");
            if (ev == SM_EVENT_PIN2) printf("[SM] Event: PIN2\n");

            // -----------------------------
            // State machine logic
            // -----------------------------
            switch (state) {

                case SM_STATE_IDLE:
                    if (ev == SM_EVENT_PIN1) {
                        printf("[SM] Transition: IDLE → ACTIVE\n");
                        state = SM_STATE_ACTIVE;
                    }
                    break;

                case SM_STATE_ACTIVE:
                    if (ev == SM_EVENT_PIN2) {
                        printf("[SM] Transition: ACTIVE → ERROR\n");
                        state = SM_STATE_ERROR;
                    }
                    break;

                case SM_STATE_ERROR:
                    if (ev == SM_EVENT_PIN1) {
                        printf("[SM] Transition: ERROR → IDLE\n");
                        state = SM_STATE_IDLE;
                    }
                    break;

                default:
                    printf("[SM] ERROR: Unknown state!\n");
                    state = SM_STATE_IDLE;
                    break;
            }
        }
    }
}

/**
 * Start the state machine demo
 * This function initializes the state machine demo by creating the event queue, configuring the GPIO pins with the appropriate interrupt handlers, and starting the state machine task.
 *  It sets up the necessary infrastructure for the state machine to function correctly, allowing it to respond to GPIO events and transition between states as defined in the sm_task function.    
 * Note: This function should be called from the main application to start the state machine demo, and it assumes that the FreeRTOS scheduler is already running.    
 * Make sure to adjust the GPIO pin numbers and debounce thresholds as needed for your specific hardware setup and requirements.    
 * The state machine logic can be expanded or modified to include additional states, events, and transitions    
 * as needed for your application, but this basic structure provides a starting point for implementing a simple state machine driven by GPIO interrupts on an ESP32 using FreeRTOS. 
 * 
 */
void start_state_machine_demo(void)
{
    // Create event queue
    sm_event_queue = xQueueCreate(10, sizeof(sm_event_t));
    if (sm_event_queue == NULL) {
        printf("[SM] ERROR: Failed to create event queue.\n");
        return;
    }

    // Configure GPIOs
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << SM_PIN1) | (1ULL << SM_PIN2),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io);

    gpio_isr_handler_add(SM_PIN1, sm_pin1_isr, NULL);
    gpio_isr_handler_add(SM_PIN2, sm_pin2_isr, NULL);

    // Create the state machine task
    xTaskCreate(sm_task, "sm_task", 4096, NULL, 7, NULL);
    printf("[SM] State machine demo initialized.\n");
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

// -----------------------------
// GPIO pins for state machine
// -----------------------------
#define SM_PIN1   25  // Rising edge triggers EVENT_PIN1
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

// -----------------------------
// ISR for PIN1
// -----------------------------
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


// -----------------------------
// ISR for PIN2
// -----------------------------
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

// -----------------------------
// State machine task
// -----------------------------
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

// -----------------------------
// Public function to start demo
// -----------------------------
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

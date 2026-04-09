/**
 * queue_demo.c - A demo of using a FreeRTOS queue to send events from a GPIO interrupt on an ESP32.
 * This demo sets up a GPIO pin with an interrupt handler that sends events to a FreeRTOS queue, and a task that waits for events from the queue and processes them.
 * - GPIO PIN_DEMO triggers an event on rising edge
 * - The event is sent to a queue from the ISR
 * - A task waits for events from the queue and prints messages when events are received
 * 
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define QUEUE_DEMO_PIN   4

typedef enum {
    QUEUE_EVENT_PIN_RISE = 1,
} queue_event_t;

static QueueHandle_t queue_demo_queue = NULL;

/**
 * ISR for GPIO pin
 * This ISR is triggered on a rising edge of the specified GPIO pin. It sends an event to the queue to notify the task that an event has occurred.    
 * Note: Make sure to configure the GPIO pin correctly with pull-up or pull-down resistors as needed, and to set up the interrupt type to trigger on the appropriate edge (e.g., rising edge) for this ISR to work correctly.    
 * The queue should be created before the ISR is triggered, and the task that waits on the queue should be running to process the events sent by this ISR for the demo to function as intended.    
 * The logic can be expanded or modified to include additional events, error handling, or timeouts as needed for your application, but this basic structure provides a starting point for implementing a simple event notification mechanism using queues with GPIO interrupts on an ESP32 using FreeRTOS.
 * 
 */ 
static void IRAM_ATTR queue_demo_isr(void *arg)
{
    queue_event_t ev = QUEUE_EVENT_PIN_RISE;
    BaseType_t hp = pdFALSE;

    // Send event to queue from ISR
    xQueueSendFromISR(queue_demo_queue, &ev, &hp);
    if (hp == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}
/**
 * Task that waits for events from the queue
 * This FreeRTOS task waits for events sent by the GPIO ISR through the queue. When it receives an event, it processes it (in this case, by printing a message to the console).    
 * The task runs indefinitely, waiting for events and processing them as they come in for demonstration purposes.    
 * Note: The queue should be created before this task is started, and the GPIO pin should be configured with the appropriate interrupt handler for this demo to function correctly.    
 * Make sure to adjust the GPIO pin numbers as needed for your specific hardware setup and requirements.    
 * The logic can be expanded or modified to include additional events, error handling, or timeouts as needed for your application, but this basic structure provides a starting point for implementing a simple event notification mechanism using queues with GPIO interrupts on an ESP32 using FreeRTOS.
 * 
 */

static void queue_demo_task(void *arg)
{
    queue_event_t ev;

    printf("[QUEUE] Task started. Tap GPIO %d to 3.3V.\n", QUEUE_DEMO_PIN);

    while (1) {
        // Block until an event arrives
        if (xQueueReceive(queue_demo_queue, &ev, portMAX_DELAY) == pdTRUE) {
            if (ev == QUEUE_EVENT_PIN_RISE) {
                printf("[QUEUE] Event: PIN RISING EDGE received via queue.\n");
            }
        }
    }
}

/**
 * Start the queue demo
 * This function initializes the queue demo by creating the queue, configuring the GPIO pin with the appropriate interrupt handler, and starting the task that waits for events from the queue.    
 * It sets up the necessary infrastructure for the demo to function correctly, allowing it to respond to GPIO events and process them through a FreeRTOS queue as defined in the queue_demo_task function.    
 * Note: This function should be called from the main application to start the demo, and it assumes that the FreeRTOS scheduler is already running.    
 * Make sure to adjust the GPIO pin numbers and debounce thresholds as needed for your specific hardware setup and requirements.    
 * The logic can be expanded or modified to include additional events, error handling, or timeouts as needed for your application, but this basic structure provides a starting point for implementing a simple event notification mechanism using queues with GPIO interrupts on an ESP32 using FreeRTOS.
 * 
 */ 

void start_queue_demo(void)
{
    // Create queue for events
    queue_demo_queue = xQueueCreate(10, sizeof(queue_event_t));
    if (queue_demo_queue == NULL) {
        printf("[QUEUE] ERROR: Failed to create queue.\n");
        return;
    }

    // Configure GPIO as input with rising edge interrupt
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << QUEUE_DEMO_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io);    
    gpio_isr_handler_add(QUEUE_DEMO_PIN, queue_demo_isr, NULL);
    xTaskCreate(queue_demo_task, "queue_demo_task", 4096, NULL, 5, NULL);
}

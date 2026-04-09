/**
 * notification_demo.c - A demo of using FreeRTOS task notifications to signal events from a GPIO interrupt on an ESP32.
 * This demo sets up a GPIO pin with an interrupt handler that sends a notification to a FreeRTOS task, and a task that waits for the notification and processes it.
 * - GPIO NOTIF_DEMO_PIN triggers an event on falling edge
 * - The event is sent as a task notification from the ISR
 * - A task waits for the notification and prints a message when the event is received
 * 
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

/**
 * ISR for GPIO pin
 * This ISR is triggered on a falling edge of the specified GPIO pin. It sends a notification   to the task to notify it that an event has occurred.    
 * Note: Make sure to configure the GPIO pin correctly with pull-up or pull-down resistors
 */
#define NOTIF_DEMO_PIN   18 // GPIO pin for notification demo

static TaskHandle_t notif_demo_task_handle = NULL; // Handle for the task that will receive notifications

/**
 * ISR for GPIO pin
 * This ISR is triggered on a falling edge of the specified GPIO pin. It sends a notification to the task to notify it that an event has occurred.    
 * Note: Make sure to configure the GPIO pin correctly with pull-up or pull-down resistors as needed, and to set up the interrupt type to trigger on the appropriate edge (e.g., falling edge) for this ISR to work correctly.    
 * The task that receives the notifications should be created before this ISR is triggered for the demo to function as intended.    
 * The logic can be expanded or modified to include additional events, error handling, or timeouts as needed for your application, but this basic structure provides a starting point for implementing a simple event notification mechanism using task notifications with GPIO interrupts on an ESP32 using FreeRTOS.
 * 
 */

static void IRAM_ATTR notif_demo_isr(void *arg)
{
    BaseType_t hp = pdFALSE;

    // Directly notify the task (no data, just a signal)
    vTaskNotifyGiveFromISR(notif_demo_task_handle, &hp);
    if (hp == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

/**
 * Task that waits for notifications
 * This FreeRTOS task waits for notifications sent by the GPIO ISR. When it receives a notification, it processes it (in this case, by printing a message to the console).    
 * The task runs indefinitely, waiting for notifications and processing them as they come in for demonstration purposes.    
 * Note: The task should be created before the ISR is triggered, and the GPIO pin should be configured with the appropriate interrupt handler for this demo to function correctly.    
 * Make sure to adjust the GPIO pin numbers as needed for your specific hardware setup and requirements.    
 * The logic can be expanded or modified to include additional events, error handling, or timeouts as needed for your application, but this basic structure provides a starting point for implementing a simple event notification mechanism using task notifications with GPIO interrupts on an ESP32 using FreeRTOS.
 * 
 */

static void notif_demo_task(void *arg)
{
    printf("[NOTIF] Task started. Tap GPIO %d to 3.3V.\n", NOTIF_DEMO_PIN);

    while (1) {
        // Block until notification is given
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        printf("[NOTIF] Signal received via task notification.\n");
    }
}

/**
 * Start the notification demo
 * This function initializes the notification demo by creating the task, configuring the GPIO pin with the appropriate interrupt handler, and starting the task that waits for notifications.    
 * It sets up the necessary infrastructure for the demo to function correctly, allowing it to respond to GPIO events and process them through FreeRTOS task notifications as defined in the notif_demo_task function.    
 * Note: This function should be called from the main application to start the demo, and it assumes that the FreeRTOS scheduler is already running.    
 * Make sure to adjust the GPIO pin numbers and debounce thresholds as needed for your specific hardware setup and requirements.    
 * The logic can be expanded or modified to include additional events, error handling, or timeouts as needed for your application, but this basic structure provides a starting point for implementing a simple event notification mechanism using task notifications with GPIO interrupts on an ESP32 using FreeRTOS.
 * 
 */

void start_notification_demo(void)
{
    // Configure GPIO as input with rising edge interrupt
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << NOTIF_DEMO_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io);
    gpio_isr_handler_add(NOTIF_DEMO_PIN, notif_demo_isr, NULL);
    xTaskCreate(notif_demo_task, "notif_demo_task", 4096, NULL, 6, &notif_demo_task_handle);
}

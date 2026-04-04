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

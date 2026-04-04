#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define NOTIF_DEMO_PIN   18

static TaskHandle_t notif_demo_task_handle = NULL;

static void IRAM_ATTR notif_demo_isr(void *arg)
{
    BaseType_t hp = pdFALSE;

    // Directly notify the task (no data, just a signal)
    vTaskNotifyGiveFromISR(notif_demo_task_handle, &hp);
    if (hp == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

static void notif_demo_task(void *arg)
{
    printf("[NOTIF] Task started. Tap GPIO %d to 3.3V.\n", NOTIF_DEMO_PIN);

    while (1) {
        // Block until notification is given
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        printf("[NOTIF] Signal received via task notification.\n");
    }
}

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

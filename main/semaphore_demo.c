#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

// -----------------------------
// GPIO pins for two events
// -----------------------------
#define PIN_X   4   // Falling edge → semaphore X
#define PIN_Y   5   // Falling edge → semaphore Y

// -----------------------------
// Two semaphores
// -----------------------------
static SemaphoreHandle_t sem_x = NULL;
static SemaphoreHandle_t sem_y = NULL;

// -----------------------------
// ISR for PIN_X
// -----------------------------
static void IRAM_ATTR isr_pin_x(void *arg)
{
    BaseType_t hp = pdFALSE;
    xSemaphoreGiveFromISR(sem_x, &hp);
    if (hp) portYIELD_FROM_ISR();
}

// -----------------------------
// ISR for PIN_Y
// -----------------------------
static void IRAM_ATTR isr_pin_y(void *arg)
{
    BaseType_t hp = pdFALSE;
    xSemaphoreGiveFromISR(sem_y, &hp);
    if (hp) portYIELD_FROM_ISR();
}

// -----------------------------
// Task that waits for X → then Y
// -----------------------------
static void two_sem_task(void *arg)
{
    printf("\n[2SEM] Task started.\n");
    printf("[2SEM] Waiting for X (GPIO %d)...\n", PIN_X);

    // Wait for first event
    xSemaphoreTake(sem_x, portMAX_DELAY);
    printf("[2SEM] Got X! Now waiting for Y (GPIO %d)...\n", PIN_Y);

    // Wait for second event
    xSemaphoreTake(sem_y, portMAX_DELAY);
    printf("[2SEM] Got Y! Sequence complete.\n");

    // End or loop forever
    while (1) {
        vTaskDelay(portMAX_DELAY);
    }
}

// -----------------------------
// Public function to start demo
// -----------------------------
void start_semaphore_demo(void)
{
    // Create semaphores
    sem_x = xSemaphoreCreateBinary();
    sem_y = xSemaphoreCreateBinary();

    if (!sem_x || !sem_y) {
        printf("[2SEM] ERROR: Failed to create semaphores.\n");
        return;
    }

    // Configure GPIOs
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << PIN_X) | (1ULL << PIN_Y),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,          // internal pull-up
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io);

    // Register ISRs
    gpio_isr_handler_add(PIN_X, isr_pin_x, NULL);
    gpio_isr_handler_add(PIN_Y, isr_pin_y, NULL);

    // Create task
    xTaskCreate(two_sem_task, "two_sem_task", 4096, NULL, 7, NULL);

    printf("[2SEM] Demo initialized.\n");
    printf("[2SEM] Tap GPIO %d for X, GPIO %d for Y.\n", PIN_X, PIN_Y);
}

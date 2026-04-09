/**
 * semaphore_demo.c - A demo of using two semaphores to synchronize tasks with GPIO interrupts on an ESP32 using FreeRTOS.
 * This demo sets up two GPIO pins with interrupt handlers that give semaphores, and a task that waits for the semaphores in a specific sequence.
 * - GPIO PIN_X triggers semaphore X        
 * - GPIO PIN_Y triggers semaphore Y    
 *              
 */

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

/**
 * ISR for PIN_X
 * This ISR is triggered on a falling edge of PIN_X. It gives the semaphore for X, and if giving the semaphore unblocks a higher priority task, it yields to allow that task to run immediately.    
 * Note: Make sure to configure the GPIO pin correctly with pull-up or pull-down resistors as needed, and to set up the interrupt type to trigger on the appropriate edge (e.g., falling edge) for this ISR to work correctly.    
 * The semaphores should be created before the ISR is triggered, and the task that waits on the semaphores should be running to process the events sent by this ISR for the demo to function as intended.       
 *  
 */
static void IRAM_ATTR isr_pin_x(void *arg)
{
    BaseType_t hp = pdFALSE;
    xSemaphoreGiveFromISR(sem_x, &hp);
    if (hp) portYIELD_FROM_ISR();
}

/**
 * ISR for PIN_Y
 * This ISR is triggered on a falling edge of PIN_Y. It gives the semaphore for Y               
 *  Note: Make sure to configure the GPIO pin correctly with pull-up or pull-down resistors as needed, and to set up the interrupt type to trigger on the appropriate edge (e.g., falling edge) for this ISR to work correctly.    
 * The semaphores should be created before the ISR is triggered, and the task that waits    on the semaphores should be running to process the events sent by this ISR for the demo to function as intended.
 * 
 */
static void IRAM_ATTR isr_pin_y(void *arg)
{
    BaseType_t hp = pdFALSE;
    xSemaphoreGiveFromISR(sem_y, &hp);
    if (hp) portYIELD_FROM_ISR();
}

/**
 * Task that waits for semaphores in sequence
 * This FreeRTOS task waits for the semaphore from PIN_X first, and once it receives it, it waits for the semaphore from PIN_Y. It prints messages to the console to indicate when it receives each event.    
 * The task runs indefinitely, waiting for the events in the specified sequence and printing the results to the console for demonstration purposes.    
 * Note: The semaphores should be created before this task is started, and the GPIO pins should be configured with the appropriate interrupt handlers for this demo to function correctly.    
 * Make sure to adjust the GPIO pin numbers as needed for your specific hardware setup and requirements.    
 * The logic can be expanded or modified to include additional events, error handling, or timeouts as needed for your application, but this basic structure provides a starting point for implementing a simple synchronization mechanism using semaphores with GPIO interrupts on an ESP32 using FreeRTOS.
 * 
 */
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

/**
 * Start the two semaphore demo
 * This function initializes the two semaphore demo by creating the semaphores, configuring the GPIO pins with the appropriate interrupt handlers, and starting the task that waits for the semaphores.    
 * It sets up the necessary infrastructure for the demo to function correctly, allowing it to respond to GPIO events and synchronize using semaphores as defined in the two_sem_task function.    
 * Note: This function should be called from the main application to start the demo, and it assumes that the FreeRTOS scheduler is already running.    
 * Make sure to adjust the GPIO pin numbers as needed for your specific hardware setup and requirements.    
 * The logic can be expanded or modified to include additional events, error handling, or timeouts as needed for your application, but this basic structure provides a starting point for implementing a simple synchronization mechanism using semaphores with GPIO interrupts on an ESP32 using FreeRTOS.
 * 
 */ 
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

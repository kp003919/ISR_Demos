#include <stdio.h>
#include "driver/gpio.h"

#include "queue_demo.h"
#include "semaphore_demo.h"
#include "notification_demo.h"
#include "state_machine_demo.h"

void app_main(void)
{
    printf("\n=== FreeRTOS Sync Demo: Queues + Semaphores + Notifications ===\n");

    // Install ISR service once
    gpio_install_isr_service(0);

    start_queue_demo();
    start_semaphore_demo();
    start_notification_demo();
    start_state_machine_demo();
}

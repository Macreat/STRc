#include "blinkStr.h"
#include "tasks.h"

void app_main(void)
{
    buttonQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(mainTask, "change_led_task", 2048, &currentLed, 10, NULL);
    setPins();
}

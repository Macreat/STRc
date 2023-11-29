#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/timers.h"
#include "driver/adc.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_intr_alloc.h"
#include "driver/ledc.h"

#define BUTTON_PIN 19
#define LED_PIN 21
#define INDICATOR_LED_PIN 2
#define cled 2
#define LED_ON 1
#define LED_OFF 0

static const char *TAG = "Button_Task";
extern bool blinking = true;

QueueHandle_t buttonQueue;

void IRAM_ATTR button_isr_handler(void *arg)
{
    int buttonState = gpio_get_level(BUTTON_PIN);
    xQueueSendFromISR(buttonQueue, &buttonState, NULL);
}
void turnOnIndicatorLed(int pin, TickType_t delayTime)
{
    gpio_set_level(pin, LED_ON);
    vTaskDelay(delayTime);
    gpio_set_level(pin, LED_OFF);
}

void blinkLed(int pin, bool blink)
{
    while (blink) // Mientras el botón no se presione
    {

        gpio_set_level(pin, LED_ON);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Encendido durante 100 ms
        gpio_set_level(pin, LED_OFF);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Apagado durante 100 ms
        int buttonState = gpio_get_level(BUTTON_PIN);
        if (buttonState == 0)
        {
            break;
        }
    }
}

void button_task(void *pvParameters)
{
    int buttonState;
    bool blink;
    while (true)
    {
        if (xQueueReceive(buttonQueue, &buttonState, portMAX_DELAY))
        {

            buttonState == 0 ? (blinking = !blinking,
                                turnOnIndicatorLed(cled, pdMS_TO_TICKS(1000)), vTaskDelay(1000 / portTICK_PERIOD_MS))
                             : ((void)0, vTaskDelay(1000 / portTICK_PERIOD_MS));

            do
            {
                ESP_LOGI(TAG, "Button pressed. Blinking... ");
                blinkLed(LED_PIN, true);
                blink = blinking;
                int bt = gpio_get_level(BUTTON_PIN);
                if (bt == 0)
                {
                    blink = false;
                }
            } while (blink); // La condición para salir del bucle
            ESP_LOGI(TAG, "Button pressed. Blinking stop. Saliendo...");
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            break;
        }
    }
}

void setPins()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_ANYEDGE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE};

    gpio_config(&io_conf);

    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_PIN) | (1ULL << INDICATOR_LED_PIN),
        .mode = GPIO_MODE_OUTPUT};

    gpio_config(&led_conf);
    
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, (void *)BUTTON_PIN);
}

void app_main(void)
{
    setPins();
    buttonQueue = xQueueCreate(1, sizeof(int));

    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
}

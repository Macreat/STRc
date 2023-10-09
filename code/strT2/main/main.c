#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "example";

#define BLINK_GPIO GPIO_NUM_23
#define BUTTON_GPIO GPIO_NUM_22
#define portTICK_PERIOD_MS 1
static uint8_t s_led_state = 0;
static bool is_blinking = false;

void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void button_task(void *pvParameter)
{
    esp_rom_gpio_pad_select_gpio(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    while (1)
    {
        if (gpio_get_level(BUTTON_GPIO) == 0)
        {
            // El botón está presionado
            if (!is_blinking)
            {
                is_blinking = true;
                ESP_LOGI(TAG, "Button pressed. LED blinking started.");
            }
            else
            {
                is_blinking = false;
                s_led_state = 0; // Detener el parpadeo apagando el LED
                ESP_LOGI(TAG, "Button pressed. LED blinking stopped.");
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Pequeña demora para evitar rebotes del botón
    }
}

void app_main(void)
{
    /* Configure the peripheral according to the LED type */
    configure_led();

    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);

    while (1)
    {
        if (is_blinking)
        {
            ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
            blink_led();
            /* Toggle the LED state */
            s_led_state = !s_led_state;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

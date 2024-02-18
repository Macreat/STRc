#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"

#define BOTON_PIN 19
#define LED1_PIN GPIO_NUM_26
#define LED2_PIN GPIO_NUM_25
#define LED3_PIN GPIO_NUM_33
#define POT_PIN ADC1_CHANNEL_7

int led_actual = 0;
int brillo_general = 0;
ledc_channel_config_t ledc_channel[3];

void configurar_pines()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BOTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_ANYEDGE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << LED1_PIN) | (1ULL << LED2_PIN) | (1ULL << LED3_PIN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);
}

void configurar_ledc()
{
    for (int i = 0; i < 3; i++)
    {
        ledc_channel[i].channel = LEDC_CHANNEL_0 + i;
        ledc_channel[i].duty = 0;
        ledc_channel[i].gpio_num = LED1_PIN + i;
        ledc_channel[i].speed_mode = LEDC_HIGH_SPEED_MODE;
        ledc_channel[i].hpoint = 0;
        ledc_channel[i].timer_sel = LEDC_TIMER_0;
        ledc_channel_config(&ledc_channel[i]);
    }

    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
    };
    ledc_timer_config(&ledc_timer);
}

void boton_isr_handler(void *arg)
{
    led_actual = (led_actual + 1) % 3;
}

void app_main(void)
{
    configurar_pines();
    configurar_ledc();

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BOTON_PIN, boton_isr_handler, (void *)BOTON_PIN);

    while (1)
    {
        for (int i = 0; i < 3; i++)
        {
            if (i == led_actual)
            {
                ledc_set_duty(LEDC_HIGH_SPEED_MODE, ledc_channel[i].channel, brillo_general);
            }
            else
            {
                ledc_set_duty(LEDC_HIGH_SPEED_MODE, ledc_channel[i].channel, 0);
            }
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, ledc_channel[i].channel);
        }

        int valor_potenciometro = adc1_get_raw(POT_PIN);
        brillo_general = (valor_potenciometro * 255) / 4095;

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

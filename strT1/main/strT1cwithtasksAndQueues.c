#include <stdio.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define Boton 19
#define Led1 GPIO_NUM_21
#define Led2 GPIO_NUM_22
#define Led3 GPIO_NUM_23

void configurar_pines()
{
    gpio_set_direction(Boton, GPIO_MODE_INPUT);
    gpio_set_direction(Led1, GPIO_MODE_OUTPUT);
    gpio_set_direction(Led2, GPIO_MODE_OUTPUT);
    gpio_set_direction(Led3, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(Boton, GPIO_PULLUP_ONLY);
}

void cambiar_led()
{
    static int led_actual = 0;

    // Cambiar al siguiente LED
    led_actual = (led_actual + 1) % 3;

    // Apagar todos los LEDs
    gpio_set_level(Led1, 0);
    gpio_set_level(Led2, 0);
    gpio_set_level(Led3, 0);

    // Encender el LED actual
    switch (led_actual)
    {
    case 0:
        gpio_set_level(Led1, 1);
        break;
    case 1:
        gpio_set_level(Led2, 1);
        break;
    case 2:
        gpio_set_level(Led3, 1);
        break;
    }
}

void boton_task(void *pvParameter)
{
    while (1)
    {
        // Leer el estado del botón
        int estado_boton = gpio_get_level(Boton);

        // Si el botón está presionado, cambiar el LED
        if (estado_boton == 0)
        {
            cambiar_led();
            vTaskDelay(500 / portTICK_PERIOD_MS); // Retardo para evitar múltiples cambios al presionar el botón
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    configurar_pines();

    // Crear tarea para manejar el botón
    xTaskCreate(boton_task, "boton_task", 2048, NULL, 10, NULL);

    // Encender el primer LED al inicio
    gpio_set_level(Led1, 1);
}

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define button 19
#define led1 21
#define led2 22
#define led3 23
#define portTICK_PERIOD_MS 1
#define ledOFF 0
#define ledON 1
#define cled 2

int currentLed = 0; // Variable para mantener el estado actual del LED


QueueHandle_t buttonQueue; // Cola para el estado del botón

void SetPines()
{
    gpio_set_direction(button, GPIO_MODE_INPUT);
    gpio_set_pull_mode(button, GPIO_PULLUP_ONLY);
    gpio_set_direction(led1, GPIO_MODE_OUTPUT);
    gpio_set_direction(led2, GPIO_MODE_OUTPUT);
    gpio_set_direction(led3, GPIO_MODE_OUTPUT);
    gpio_set_direction(cled, GPIO_MODE_OUTPUT);
}

void blinkLed(int pin)
{
    bool blinking = true; // definir al principio
    while (blinking)
    {
        int estadoButton = gpio_get_level(button);
        gpio_set_level(pin, ledON);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(pin, ledOFF);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        if (estadoButton == 0)
        {
            blinking = false;
        }
    }
}

void turnOnIndicatorLed(int pin, TickType_t delayTime)
{
    gpio_set_level(pin, ledON);
    vTaskDelay(delayTime);
    gpio_set_level(pin, ledOFF);
}

void buttonTask(void *pvParameter)
{
    while (true)
    {
        int estadoButton = gpio_get_level(button);
        xQueueSend(buttonQueue, &estadoButton, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(10)); // Pequeño retardo para evitar lecturas múltiples del botón
    }
}

void changeLed(void *pvParameters)
{
    int estadoButton;

    while (true)
    {
        xQueueReceive(buttonQueue, &estadoButton, portMAX_DELAY);

        if (estadoButton == 0)
        {
            // Enciende el indicador LED durante 2 segundos
            turnOnIndicatorLed(cled, pdMS_TO_TICKS(1000));

            // Espera un poco para evitar múltiples detecciones del botón
            vTaskDelay(pdMS_TO_TICKS(100));

            // Cambia al siguiente LED
            currentLed = *((int *)pvParameters);
            currentLed = (currentLed + 1) % 3;
            *((int *)pvParameters) = currentLed;
        }

        // Enciende el LED correspondiente
        switch (*((int *)pvParameters))
        {
        case 0:
            blinkLed(led1);
            break;
        case 1:
            blinkLed(led2);
            break;
        case 2:
            blinkLed(led3);
            break;
        }
    }
}
void app_main(void)
{
    SetPines();

    // Crear la cola para el estado del botón
    buttonQueue = xQueueCreate(1, sizeof(int));

    // Crear tareas
    xTaskCreate(buttonTask, "button_task", 2048, NULL, 10, NULL);
    xTaskCreate(changeLed, "change_led_task", 2048, &currentLed, 10, NULL);
}
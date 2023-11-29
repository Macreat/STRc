#include <stdio.h>
#include "driver/gpio.h" //Librería para configurar los GPIO
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"  //Librería para usar tareas
#include "freertos/queue.h" //Librería para usar colas
#include <esp_log.h>        //librería para poder imprimir texto con colores diferentes

#define button_isr 19   // GPIO en donde va a estar el botón que genera la interrupción
#define ledR 21         // GPIO en donde está el LED rojo (el del blink)
#define ledA 2          // GPIO en donde está el LED amarillo (el que va a indicar cuando haya una interrupción)
#define STACK_SIZE 1024 // valor de espacio de memoria para las tareas (si se pone un valor muy pequeño se va a reiniciar el uC)
#define R_delay 200     // Retardo para encendido y apagado del LED

QueueHandle_t GlobalQueue = 0; // Variable en donde se va a almacenar la cola, tiene que ser de tipo QueueHandle_t porque así lo pide la librería

// Protitipado de funciones
esp_err_t init_led(void);
esp_err_t init_isr(void);
esp_err_t create_task(void);
void state_change(void *args);
void BlinkLed(void *pvParameters);

void app_main(void)
{
    GlobalQueue = xQueueCreate(10, sizeof(uint32_t)); // Se crea la cola con espacio para 10 valores y que van a ser de tipo uint32_t
    init_led();                                       // Se configura el LED
    init_isr();                                       // Se configuran las interrupciones, en este caso es solamente una
    create_task();                                    // Crea las tareas
}

esp_err_t init_led(void) // función para configurar el LED como salida
{
    gpio_reset_pin(ledR);                       // Se resetea el GPIO a sus valores predeterminados
    gpio_set_direction(ledR, GPIO_MODE_OUTPUT); // Se indica que el GPIO se va a utilizar como salida
    gpio_reset_pin(ledA);                       // Se resetea el GPIO a sus valores predeterminados
    gpio_set_direction(ledA, GPIO_MODE_OUTPUT); // Se indica que el GPIO se va a utilizar como salida

    return ESP_OK;
}

esp_err_t init_isr(void) // Configuración e inicialización de las interrupciones
{
    gpio_config_t pGPIOConfig;                        // Prototipado de variable de tipo gpio_config_t (lo exigue la librería) utilizada para la posterior configuración
    pGPIOConfig.pin_bit_mask = (1ULL << button_isr);  // Se cra la mascara del GPIO
    pGPIOConfig.mode = GPIO_MODE_INPUT;               // Se indica que el GPIO va a estar en modo de entrada
    pGPIOConfig.pull_up_en = GPIO_PULLUP_DISABLE;     // Se deshabilita el pullup porque se implementó de forma física
    pGPIOConfig.pull_down_en = GPIO_PULLDOWN_DISABLE; // Se deshabilida el pulldoun por la misma razón anterior
    pGPIOConfig.intr_type = GPIO_INTR_POSEDGE;        // Se indica qué tipo de interrupción se va a utilizar (rissing/flanco de subida en este caso)

    gpio_config(&pGPIOConfig); // Se carga a la función gpio_config(), la variable pGPIOConfig que fue prototipada anterioremente

    gpio_install_isr_service(0);                          // Se inicia la interrupción 0
    gpio_isr_handler_add(button_isr, state_change, NULL); /* Se define cómo va a trabajar la interrupción
                                                           gpio_isr_handler_add(pin a utilizar, función que va a ejecutar, NULL)*/

    return ESP_OK;
}

esp_err_t create_task(void) // Función en donde se configuran las tareas
{
    static uint8_t ucParameterToPass; // variable para pasar parámetros a la tarea
    TaskHandle_t xHandle = NULL;      // Identificador de la tarea, en este caso como es uno solo, lo dejamos NULL

    xTaskCreate(BlinkLed, "vTaskR", STACK_SIZE, &ucParameterToPass, 1, &xHandle); /*Se crea la tarea con las siguiente caracteristicas
                                                                                    xTaskCreate(función que va a ejecutar,
                                                                                                nombre de la tarea,
                                                                                                tamaño en bit para la tarea,
                                                                                                parámetro a enviar,
                                                                                                prioridad,
                                                                                                identificador)*/

    return ESP_OK;
}

void BlinkLed(void *pvParameters) // Función que ejecuta la tarea
{
    static bool state = true; // Variable static de tipo bool para varias los estados de funcionamiento
    while (1)
    {
        int receibedValue = 0;                                                        // Se crea variable para almacenar el valor leido de la cola
        int lecture = xQueueReceive(GlobalQueue, &receibedValue, pdMS_TO_TICKS(100)); // lecture almacena true o false dependiendo si logra leer un valor o no

        if (lecture) // Si lee un valor (es decir, si al presionar el botón se agregó un valor a la cola)
        {
            state = !state; // cambia el estado
        }

        // De acuerdo con el estado realiza la respectiva acción
        switch (state)
        {
        case true:
            gpio_set_level(ledR, 1);
            vTaskDelay(pdMS_TO_TICKS(R_delay));
            gpio_set_level(ledR, 0);
            vTaskDelay(pdMS_TO_TICKS(R_delay));
            break;

        default:
            gpio_set_level(ledR, 0);
            break;
        }
    }
}

void state_change(void *args) // esta es la función que va a ejercuar cuando se presente una interrupción
{
    int state_button = gpio_get_level(button_isr);              // Leer y guarda el estado lógico del LED en state_button
    xQueueSend(GlobalQueue, &state_button, pdMS_TO_TICKS(100)); // Agrega a la cola la información del puntero state_button
}
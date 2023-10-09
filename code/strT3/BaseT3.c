#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "driver/gpio.h" //Librería para configurar los GPIO
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"         //librería para poder imprimir texto con colores diferentes
#include "freertos/timers.h" //Librería para configurar los timers
#include "driver/adc.h"      //Librería para configurar el ADC
#include "freertos/queue.h"
#include "driver/uart.h"

#define NTC_PIN ADC1_CHANNEL_7
#define ledUART 2
#define ledR 33
#define ledG 25
#define ledB 26
#define SIZE_BUFFER_TASK 1024 * 2 // valor de espacio de memoria para las tareas (si se pone un valor muy pequeño se va a reiniciar el uC)
#define Delay_Task_Switch_LED 1000
#define R_FIXED 100000
#define R0_NTC 100000
#define Beta 4190
#define Temp0 298.15
#define Vol_REF 3.3
#define UART_NUM UART_NUM_2
#define BUF_SIZE 1024

#define Init_Min_ledB 0
#define Init_Max_ledB 28
#define Init_Min_ledG 20
#define Init_Max_ledG 30
#define Init_Min_ledR 30
#define Init_Max_ledR 50

typedef struct
{
    char *key;
    int value;
} Dict_set_values;

static const char *tag = "ADC"; // tipo de variable que se tiene que crear para la librería esp_log.h (revisar documentación de la librería)
static const char *tag_UART = "UART EVENT";
static const char *tag_task = "Task";
QueueHandle_t ADC_lecture = 0;
QueueHandle_t uart_queue = 0;
QueueHandle_t Set_values_queue = 0;
Dict_set_values Values_set_min_max_LEDs;

// Prototipado de funciones
esp_err_t init_led(void);
esp_err_t set_timer(void);
esp_err_t set_adc(void);
esp_err_t create_task(void);
esp_err_t init_uart(void);
void get_ADC(void *pvParameters);
void switch_LED(void *pvParameters);
void uart_task(void *pvParameters);

void get_ADC(void *pvParameters)
{
    while (1)
    {
        int adc_val = 0;
        adc_val = adc1_get_raw(NTC_PIN); // Funcion para leer el ADC. En este caso solamente nos pide el canal que deseamos leer
        float adc_value = (float)adc_val;
        float Vol_NTC = (Vol_REF * adc_value) / 4095;
        float R_NTC = R_FIXED / ((Vol_REF / Vol_NTC) - 1);
        // float Temperatura_Kelvin = Beta/(log(R_NTC/R0_NTC)+(Beta/Temp0));
        float Temperatura_Kelvin = 1 / ((log(R_NTC / R0_NTC) / Beta) + (1 / Temp0));
        float Temperatura_Celcius = Temperatura_Kelvin - 273.15;

        xQueueSend(ADC_lecture, &Temperatura_Celcius, pdMS_TO_TICKS(50)) ?: printf("\x1b[31mError writing in queue\x1b[0m\n");

        // ESP_LOGI(tag, "Lectura: %i, VOLTAJE: %f, R_NTC: %f, TEMPERATURA: %f", adc_val, Vol_NTC, R_NTC, Temperatura_Celcius);

        vTaskDelay(pdMS_TO_TICKS(Delay_Task_Switch_LED / 5));
    }
}

void switch_LED(void *pvParameters)
{
    char *Mensaje_guia = "Comandos permitido:\n LED_X_MAX=NUM\n LED_X_MIN=NUM\nEn donde: X->R,G o B\tNUM->Número";

    int Max_ledB = Init_Max_ledB;
    int Min_ledB = Init_Min_ledB;
    int Max_ledG = Init_Max_ledG;
    int Min_ledG = Init_Min_ledG;
    int Max_ledR = Init_Max_ledR;
    int Min_ledR = Init_Min_ledR;

    while (1)
    {
        float receibedValue = 0, Sumatoria_temp = 0;

        for (size_t i = 0; i < 5; i++)
        {
            Sumatoria_temp += xQueueReceive(ADC_lecture, &receibedValue, pdMS_TO_TICKS(200)) ? receibedValue : printf("\x1b[31mError receiving value from queue\x1b[0m\n");
        }
        float Temperatura = Sumatoria_temp / 5;

        Dict_set_values values;
        if (xQueueReceive(Set_values_queue, &values, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            // Imprimimos los valores
            // printf("Key: %s\n", values.key);
            // printf("Value: %i\n", values.value);

            char *KEY = (const char *)values.key;
            int contador = 0;

            !strcmp(KEY, "LED_B_MAX") ? Max_ledB = values.value, uart_write_bytes(UART_NUM_2, "Valor máximo de LED BLUE asignado", strlen("Valor máximo de LED BLUE asignado")) : contador++;
            !strcmp(KEY, "LED_B_MIN") ? Min_ledB = values.value, uart_write_bytes(UART_NUM_2, "Valor mínimo de LED BLUE asignado", strlen("Valor mínimo de LED BLUE asignado")) : contador++;
            !strcmp(KEY, "LED_G_MAX") ? Max_ledG = values.value, uart_write_bytes(UART_NUM_2, "Valor máximo de LED GREEN asignado", strlen("Valor máximo de LED GREEN asignado")) : contador++;
            !strcmp(KEY, "LED_G_MIN") ? Min_ledG = values.value, uart_write_bytes(UART_NUM_2, "Valor mínimo de LED GREEN asignado", strlen("Valor mínimo de LED GREEN asignado")) : contador++;
            !strcmp(KEY, "LED_R_MAX") ? Max_ledR = values.value, uart_write_bytes(UART_NUM_2, "Valor máximo de LED RED asignado", strlen("Valor máximo de LED RED asignado")) : contador++;
            !strcmp(KEY, "LED_R_MIN") ? Min_ledR = values.value, uart_write_bytes(UART_NUM_2, "Valor mínimo de LED RED asignado", strlen("Valor mínimo de LED RED asignado")) : contador++;

            contador == 6 ? uart_write_bytes(UART_NUM_2, Mensaje_guia, strlen(Mensaje_guia)) : NULL;
        }

        Temperatura >= Min_ledB &&Temperatura <= Max_ledB ? gpio_set_level(ledB, 1) : gpio_set_level(ledB, 0);
        Temperatura >= Min_ledG &&Temperatura <= Max_ledG ? gpio_set_level(ledG, 1) : gpio_set_level(ledG, 0);
        Temperatura >= Min_ledR &&Temperatura <= Max_ledR ? gpio_set_level(ledR, 1) : gpio_set_level(ledR, 0);

        ESP_LOGI(tag, "Temperatura= %.1f °C", Temperatura);
        vTaskDelay(pdMS_TO_TICKS(Delay_Task_Switch_LED));
    }
}

void uart_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

    while (1)
    {
        if (xQueueReceive(uart_queue, (void *)&event, portMAX_DELAY))
        {
            bzero(data, BUF_SIZE);

            switch (event.type)
            {
            case UART_DATA:
                uart_read_bytes(UART_NUM, data, event.size, pdMS_TO_TICKS(500));
                uart_write_bytes(UART_NUM, (const char *)data, event.size);
                uart_flush(UART_NUM);

                char *ptr = strrchr((const char *)data, '=');

                if (ptr != NULL)
                {
                    char *data_whitout_LB = strtok((const char *)data, "\n");
                    // printf("data_whitout_LB= %s\n", data_whitout_LB);

                    char *data_key = strtok(data_whitout_LB, "=");
                    Values_set_min_max_LEDs.key = data_key;
                    // printf("Key= %s\n", Values_set_min_max_LEDs.key);

                    char *data_value = strtok(NULL, "=");
                    int data_value_int = atoi(data_value);
                    Values_set_min_max_LEDs.value = data_value_int;
                    // printf("Value= %d\n", Values_set_min_max_LEDs.value);

                    !xQueueSend(Set_values_queue, &Values_set_min_max_LEDs, pdMS_TO_TICKS(100)) ? printf("Error cargando valor a la cola\n") : NULL;
                }
                else
                {
                    ESP_LOGE(tag_UART, "Estructura de entrada incorrecta!");
                }

                break;

            default:
                break;
            }
        }
    }
}

void app_main(void)
{
    ADC_lecture = xQueueCreate(10, sizeof(float));
    Set_values_queue = xQueueCreate(6, sizeof(Dict_set_values));
    init_led();
    set_adc();
    init_uart();
    create_task();
}

esp_err_t init_uart(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .parity = UART_PARITY_DISABLE,
        .source_clk = UART_SCLK_APB,
        .stop_bits = UART_STOP_BITS_1};

    uart_param_config(UART_NUM, &uart_config);

    uart_set_pin(UART_NUM, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_NUM, BUF_SIZE, BUF_SIZE, 5, &uart_queue, 0);

    ESP_LOGI(tag_UART, "Init uart completed");

    return ESP_OK;
}

esp_err_t init_led(void) // función para configurar el LED como salida
{
    gpio_reset_pin(ledUART);
    gpio_set_direction(ledUART, GPIO_MODE_OUTPUT);

    gpio_reset_pin(ledR);
    gpio_set_direction(ledR, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledG);
    gpio_set_direction(ledG, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledB);
    gpio_set_direction(ledB, GPIO_MODE_OUTPUT);
    ESP_LOGI(tag, "LED's inicialized");
    return ESP_OK;
}

esp_err_t create_task(void) // Función en donde se configuran las tareas
{
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    xTaskCreate(get_ADC,
                "get_ADC",
                SIZE_BUFFER_TASK,
                &ucParameterToPass,
                2,
                &xHandle);

    xTaskCreate(switch_LED,
                "switch_LED",
                SIZE_BUFFER_TASK,
                &ucParameterToPass,
                1,
                &xHandle);

    xTaskCreate(uart_task,
                "uart_task",
                SIZE_BUFFER_TASK,
                &ucParameterToPass,
                5,
                &xHandle);
    ESP_LOGI(tag_task, "Tasks created");

    return ESP_OK;
}

esp_err_t set_adc(void)
{
    adc1_config_channel_atten(NTC_PIN, ADC_ATTEN_DB_11); // Aquí se escoge el canar a utilizar y la ateniación que deseamos de acuerdo a nuestra señal
    adc1_config_width(ADC_WIDTH_BIT_12);                 // Aquí se escoge la resolución que deseamos para el ADC
    ESP_LOGI(tag, "ADC configured");
    return ESP_OK;
}
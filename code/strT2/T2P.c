#include <stdio.h>
#include "driver/ledc.h"
#include "esp_log.h" //librería para poder imprimir texto con colores diferentes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h" //Librería para configurar los timers
#include "driver/adc.h"      //Librería para configurar el ADC
#include "driver/gpio.h"     //Librería para configurar los GPIO

#define STACK_SIZE 1024 * 2 // valor de espacio de memoria para las tareas (si se pone un valor muy pequeño se va a reiniciar el uC)
#define ledR 23             // Número de GPIO para el LED rojo
#define ledG 22             // Número de GPIO para el LED verde
#define ledY 21             // Número de GPIO para el LED amarillo

// Prototipado se funciones
esp_err_t set_pwm(void);
esp_err_t set_pwm_duty(int adcR, int adcG, int adcB);
esp_err_t set_adc(void);
esp_err_t create_task(void);
void fadeXLed();

static const char *tag = "Main"; // tipo de variable que se tiene que crear para la librería esp_log.h (revisar documentación de la librería)
int interval = 10;               // Tiempo que va a tardar en ejecutarse el timer
int timerId = 0;                 // Identificación de cada timer, en este ejemplo no fue tan util ya que solamente hay uno
bool buttonPressed = false;
#define button_isr 19 // GPIO en donde va a estar el botón que genera la interrupción

// prototipado de funciones
esp_err_t init_isr(void);
esp_err_t led_change(void);

int count = 0;   // variable para controlar cuál led se enciende
int adc_val = 0; // variable para guardar el valor de lectura del acd

esp_err_t create_task(void) // Función en donde se configuran las tareas
{
    static uint8_t ucParameterToPass; // variable para pasar parámetros a la tarea
    TaskHandle_t xHandle = NULL;      // Identificador de la tarea, en este caso como es uno solo, lo dejamos NULL

    xTaskCreate(fadeXLed, "vTaskR", STACK_SIZE, &ucParameterToPass, 1, &xHandle); /*Se crea la tarea con las siguiente caracteristicas
                                                                                    xTaskCreate(función que va a ejecutar,
                                                                                                nombre de la tarea,
                                                                                                tamaño en bit para la tarea,
                                                                                                parámetro a enviar,
                                                                                                prioridad,
                                                                                                identificador)*/

    return ESP_OK;
}

esp_err_t set_pwm(void) // Función para configurar el PWM
{
    // Se crea una variable de tipo ledc_channel_config_t vacía y después se asignan los parámetros
    // que se requiere. Estos se pueden revisar en la librería. Se hace el mismo procedimiento para
    // Los 3 LEDs
    ledc_channel_config_t channelConfigR = {0};
    channelConfigR.gpio_num = ledR;
    channelConfigR.speed_mode = LEDC_HIGH_SPEED_MODE;
    channelConfigR.channel = LEDC_CHANNEL_0;
    channelConfigR.intr_type = LEDC_INTR_DISABLE;
    channelConfigR.timer_sel = LEDC_TIMER_1;
    channelConfigR.duty = 0;

    ledc_channel_config_t channelConfigG = {0};
    channelConfigG.gpio_num = ledG;
    channelConfigG.speed_mode = LEDC_HIGH_SPEED_MODE;
    channelConfigG.channel = LEDC_CHANNEL_1;
    channelConfigG.intr_type = LEDC_INTR_DISABLE;
    channelConfigG.timer_sel = LEDC_TIMER_1;
    channelConfigG.duty = 0;

    ledc_channel_config_t channelConfigY = {0};
    channelConfigY.gpio_num = ledY;
    channelConfigY.speed_mode = LEDC_HIGH_SPEED_MODE;
    channelConfigY.channel = LEDC_CHANNEL_2;
    channelConfigY.intr_type = LEDC_INTR_DISABLE;
    channelConfigY.timer_sel = LEDC_TIMER_1;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
    channelConfigY.duty = 0;

    // Se configura cada canal de PWM a utilizar. Para esto, con ayuda del puntero enviamos toda
    // la información que está guardada en cada variable de las que creamos hace un momento.
    // Tiene que ser ese tipo de variable ya que es la que pide la librería (ledc.h).
    ledc_channel_config(&channelConfigR);
    ledc_channel_config(&channelConfigG);
    ledc_channel_config(&channelConfigY);

    // Se se crea una variable de tipo ledc_timer_config_t vacía y despues se asignan los parámetros
    // Esto se hace para despues configurar la resolución con la que va a trabajar el timer del PWM.
    // Para este caso se Trabajó con una resolución de 10 bits y una frecuencia de 20Khz
    ledc_timer_config_t timerConfig = {0};
    timerConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
    timerConfig.duty_resolution = LEDC_TIMER_10_BIT;
    timerConfig.timer_num = LEDC_TIMER_1;
    timerConfig.freq_hz = 20000; // 20Khz

    // Aquí se toma la dirección de memoría de la variable configurada anteriormente y se ingresa
    // en la función ledc_timer_config para configurar el timer del PWM
    ledc_timer_config(&timerConfig);

    return ESP_OK;
}

esp_err_t set_pwm_duty(int adcR, int adcG, int adcY)
{
    // Configura el ciclo últil de cada led de acuerdo con su variable
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, adcR / 4);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, adcG / 4);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, adcY / 4);

    // Actualiza el ciclo últil, es decir, lo envía hacia el led porque en el paso anterior
    // Solamente guarda la variable para ser enviada
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);

    return ESP_OK;
}

esp_err_t set_adc(void)
{
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11); // Aquí se escoge el canar a utilizar y la ateniación que deseamos de acuerdo a nuestra señal
    adc1_config_width(ADC_WIDTH_BIT_12);                        // Aquí se escoge la resolución que deseamos para el ADC
    return ESP_OK;
}

void fadeXLed() // Funcion asignada a la tarea. Se encargar de leer el ADC y controlar cuál LED se enciende
{
    while (1)
    {
        adc_val = adc1_get_raw(ADC1_CHANNEL_7); // Funcion para leer el ADC. En este caso solamente nos pide el canal que deseamos leer
        switch (count)
        {
        case 0:

            set_pwm_duty(adc_val, 0, 0);

            ESP_LOGE(tag, "ADC VAL: %i  PORCE_ON: %i    ON: ROJO", adc_val, (adc_val * 100) / 4095);
            break;
        case 1:

            set_pwm_duty(0, adc_val, 0);
            ESP_LOGI(tag, "ADC VAL: %i  PORCE_ON: %i    ON: VERDE", adc_val, (adc_val * 100) / 4095);
            break;
        case 2:

            set_pwm_duty(0, 0, adc_val);
            ESP_LOGW(tag, "ADC VAL: %i  PORCE_ON: %i    ON: AMARILLO", adc_val, (adc_val * 100) / 4095);
            break;
        default:
            set_pwm_duty(0, 0, 0);
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
esp_err_t led_change(void)
{
    if (gpio_get_level(button_isr) == 1)
    {
        buttonPressed = true;
        count++;
        if (count > 2)
        {
            count = 0;
        }
    }
    else
    {
        buttonPressed = false;
    }
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

    gpio_install_isr_service(0);                        // Se inicia la interrupción 0
    gpio_isr_handler_add(button_isr, led_change, NULL); /* Se define cómo va a trabajar la interrupción
                                                           gpio_isr_handler_add(pin a utilizar, función que va a ejecutar, NULL)*/

    return ESP_OK;
}

void app_main(void)
{
    set_pwm();     // Se configura el PWM para cada LED
    init_isr();    // Se configuran las interrupciones, en este caso es solamente una
    create_task(); // Crea las tareas
}

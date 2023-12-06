    #include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define LM35_ADC_CHANNEL ADC1_CHANNEL_7 // Esto es para GPIO34, puedes cambiarlo según tu conexión
#define DEFAULT_VREF 1100               // Uso el Vref por defecto para la calibración

void app_main()
{
    // Inicializar ADC
    adc1_config_width(ADC_WIDTH_BIT_12);                          // Configuración de 12 bits
    adc1_config_channel_atten(LM35_ADC_CHANNEL, ADC_ATTEN_DB_11); // Atenuación para entrada máxima de 3.3V

    while (1)
    {
        // Leer ADC
        int adc_reading = adc1_get_raw(LM35_ADC_CHANNEL);

        // Convertir lectura ADC a temperatura en °C
        float voltage = (float)adc_reading / 4095.0 * 3.3; // Convertir lectura a voltaje (3.3V es la referencia)
        float temperature = voltage * 100.0;               // LM35 da 10 mV por °C

        printf("Temperatura: %.2f°C\n", temperature);

        vTaskDelay(1000 / porTICK_PERIOD_MS); // Esperar 1 segundo
    }
}
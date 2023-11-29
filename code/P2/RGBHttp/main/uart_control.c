#include "uart_control.h"
#include "rgb_led.h" // Asegúrate de incluir el encabezado para la función updateRGB

// Inicialización de UART
void init_uart(void)
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

    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

    ESP_LOGI(tag, "UART initialized");
}

// Función para leer datos de UART y actualizar los LEDs RGB
void update_leds_from_uart(void)
{
    uint8_t data[BUF_SIZE];
    int length = 0;
    while (1)
    {
        // Lee los datos de UART
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM, (size_t *)&length));
        length = uart_read_bytes(UART_NUM, data, length, 100 / portTICK_PERIOD_MS);

        // Procesa los datos si se recibió algo
        if (length > 0)
        {
            data[length] = 0; // Asegurarse de que los datos recibidos sean una cadena válida
            int r, g, b;
            // Intenta interpretar los datos recibidos como valores RGB
            if (sscanf((const char *)data, "R%dG%dB%d", &r, &g, &b) == 3)
            {
                // Actualiza los valores del LED RGB
                updateRGB(r, g, b);
                ESP_LOGI(tag, "LED updated to R: %d, G: %d, B: %d", r, g, b);
            }
            else
            {
                ESP_LOGI(tag, "Invalid data received");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Pequeña pausa para no saturar el CPU
    }
}

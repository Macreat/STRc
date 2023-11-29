#include "ButtonTask.h"

// Definiciones de los pines de los botones
#define BUTTON_1_GPIO 18
#define BUTTON_2_GPIO 19

// Variables para almacenar las pulsaciones de los botones
extern volatile int button_1_press_count = 0;
extern volatile int button_2_press_count = 0;

// ISR handler para el botón 1
static void IRAM_ATTR button_1_isr_handler(void *arg)
{
    button_1_press_count++;
}

// ISR handler para el botón 2
static void IRAM_ATTR button_2_isr_handler(void *arg)
{
    button_2_press_count++;
}

// Función para inicializar la tarea de botones
void button_task_initialize(void)
{
    // Configurar los pines GPIO como entradas con pull-up
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = (1ULL << BUTTON_1_GPIO) | (1ULL << BUTTON_2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0};
    gpio_config(&io_conf);

    // Instalar el servicio ISR y agregar las interrupciones para cada botón
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_1_GPIO, button_1_isr_handler, (void *)BUTTON_1_GPIO);
    gpio_isr_handler_add(BUTTON_2_GPIO, button_2_isr_handler, (void *)BUTTON_2_GPIO);
}

int get_button_1_press_count()
{
    return button_1_press_count;
}

int get_button_2_press_count()
{
    return button_2_press_count;
}

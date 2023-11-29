
#ifndef PROTOCOL_EXAMPLES_COMMON_H
#define PROTOCOL_EXAMPLES_COMMON_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Define las constantes para el éxito y el fracaso de la conexión Wi-Fi
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1

    // Funciones para inicializar la memoria no volátil y establecer la conexión Wi-Fi
    esp_err_t initialize_nvs(void);
    esp_err_t connect_wifi(const char *ssid, const char *password);

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_EXAMPLES_COMMON_H */
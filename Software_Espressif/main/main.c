#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "Control.h"
#include "Interface.h"
#include "Sensor.h"

void init();

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

void app_main(void)
{
	init();
}

void init()
{
	Sensor_init();
	Control_init();
	Interface_init();
}

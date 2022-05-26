/*
 * Interface.c
 *
 *  Created on: 2022. 5. 25.
 *      Author: Gyuhyun_Cho
 */
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "math.h"

#include "WebPage.h"

#include "Parameter.h"

#define LOG "WIFI"

httpd_handle_t server;
httpd_uri_t uri_main;

static int control;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(LOG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(LOG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

int Interface_init()
{
	int ret_val = 1;

	control = 5;

	esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
    	ESP_ERROR_CHECK(nvs_flash_erase());
    	ret = nvs_flash_init();
    }

	esp_netif_init();
	esp_event_loop_create_default();
	esp_netif_create_default_wifi_ap();

	wifi_init_config_t wifi_config_default = WIFI_INIT_CONFIG_DEFAULT();

	esp_wifi_init(&wifi_config_default);

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wf_config =
	{
		.ap =
		{
			.ssid = ESP_WIFI_SSID,
			.ssid_len = strlen(ESP_WIFI_SSID),
			.channel = ESP_WIFI_CHANNEL,
			.password = ESP_WIFI_PASS,
			.max_connection = ESP_MAX_STA_CONN,
			.authmode = WIFI_AUTH_WPA_WPA2_PSK
		},
	};

	if( esp_wifi_set_mode(WIFI_MODE_AP) == ESP_OK && esp_wifi_set_config(WIFI_IF_AP, &wf_config) == ESP_OK )
	{
		if ( esp_wifi_start() == ESP_OK )
		{
			ret_val = 0;
		}
	}

#ifdef DEBUG
	if (ret_val == 0) ESP_LOGI(LOG, "Wifi Init successful!");
	else ESP_LOGE(LOG, "Wifi Init failed!");
#endif

	return ret_val;
}

int Interface_main()
{
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	int ret_val = 0;

	if ( httpd_start(&server, &config) == ESP_OK )
	{
		httpd_register_uri_handler(server, &uri_main);
		ret_val = 1;
	}

	return ret_val;
}

int get_main(httpd_req_t *req)
{
	httpd_resp_send_chunk(req, Html_Main, strlen(Html_Main));
	httpd_resp_send_chunk(req, NULL, 0);

	int qur_len = httpd_req_get_url_query_len(req) + 1;
	char * qur;

	int input=2;

#ifdef DEBUG
			ESP_LOGI(LOG, "QUR_len : %d", qur_len);
#endif
	if (qur_len >1)
	{
		qur = malloc(qur_len);
		if (httpd_req_get_url_query_str(req, qur, qur_len) == ESP_OK)
		{
#ifdef DEBUG
			ESP_LOGI(LOG, "QUR : %s", qur);
#endif
   			if ( httpd_query_key_value(qur, "CON", input, sizeof(input)) == ESP_OK )
   			{
   				if (input == 0)
   				{
   					if(input > WHEEL_MIN)
   						control--;
   				}
   				else if (input == 1)
   				{
   					if(input < WHEEL_MAX)
   						control++;
   				}
#ifdef DEBUG
   				ESP_LOGI(LOG, "CON : %d", charToInt(input,1));
#endif
   			}
		}
	}

	return 0;
}

httpd_uri_t uri_main = {
		.uri	= "/main",
		.method = HTTP_GET,
		.handler = get_main,
		.user_ctx = NULL
};

/*
 * Query handler helper functions
 */
int charToInt(char *buf, int limit)
{
	int ret_val = 0;
	for ( int i = 0 ; i < limit ; i++ )
	{
		ret_val = ret_val + ( buf[i] - '0' ) * pow( (double)10, ( limit - i - 1 ) );
	}
#ifdef DEBUG
	ESP_LOGI(LOG, "Retval : %d", ret_val);
#endif

	return ret_val;
}

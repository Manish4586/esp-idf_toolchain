
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"



static const char *TAG = "scan";


static void test_wifi_scan_all()
{
	uint16_t ap_count = 0;
	wifi_ap_record_t *ap_list;
	uint8_t i;
	char *authmode;

	esp_wifi_scan_get_ap_num(&ap_count);	
	printf("--------scan count of AP is %d-------\n", ap_count);
	if (ap_count <= 0)
		return; 

	ap_list = (wifi_ap_record_t *)malloc(ap_count * sizeof(wifi_ap_record_t));
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_list));	

	printf("======================================================================\n");
	printf("             SSID             |    RSSI    |           AUTH           \n");
	printf("======================================================================\n");
    for (i = 0; i < ap_count; i++) 
	{
	    switch(ap_list[i].authmode) 
	    {
		    case WIFI_AUTH_OPEN:
		       authmode = "WIFI_AUTH_OPEN";
		       break;
		    case WIFI_AUTH_WEP:
		       authmode = "WIFI_AUTH_WEP";
		       break;           
		    case WIFI_AUTH_WPA_PSK:
		       authmode = "WIFI_AUTH_WPA_PSK";
		       break;           
		    case WIFI_AUTH_WPA2_PSK:
		       authmode = "WIFI_AUTH_WPA2_PSK";
		       break;           
		    case WIFI_AUTH_WPA_WPA2_PSK:
		       authmode = "WIFI_AUTH_WPA_WPA2_PSK";
		       break;
		    default:
		       authmode = "Unknown";
		       break;
    	        }
   		printf("%26.26s    |    % 4d    |    %22.22s\n", ap_list[i].ssid, ap_list[i].rssi, authmode);
	}
        free(ap_list);
}


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            //ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            //ESP_LOGI(TAG, "Got IP: %s\n",
            //         ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            //ESP_ERROR_CHECK(esp_wifi_connect());
            break;
		case SYSTEM_EVENT_SCAN_DONE:
			ESP_LOGI(TAG, "SYSTEM_EVENT_SCAN_DONE");
			test_wifi_scan_all();
			break;
        default:
            break;
    }
    return ESP_OK;
}

/* Initialize Wi-Fi as sta and set scan method */
static void wifi_scan(void)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_scan_config_t scan_config = {
		.ssid = 0,
		.bssid = 0,
		.channel = 0,	/* 0--all channel scan */
		.show_hidden = 1,
		.scan_type = WIFI_SCAN_TYPE_ACTIVE,
		.scan_time.active.min = 120,
		.scan_time.active.max = 150,
	};

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    //ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
	while (1)
	{
		ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    wifi_scan();
}
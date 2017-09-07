#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt.h"
#include "cJSON.h"

const char *MQTT_TAG = "MQTT_SAMPLE";

static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

void connected_cb(void *self, void *params)
{
    mqtt_client *client = (mqtt_client *)self;
    mqtt_subscribe(client, "devices/gateway1/sub", 0);
    mqtt_publish(client, "devices/gateway1/pub", "howdy!", 6, 0, 0);
}
void disconnected_cb(void *self, void *params)
{

}
void reconnect_cb(void *self, void *params)
{

}
void subscribe_cb(void *self, void *params)
{
    ESP_LOGI(MQTT_TAG, "[APP] Subscribe ok, test publish msg");
    mqtt_client *client = (mqtt_client *)self;
    mqtt_publish(client, "devices/gateway1/pub", "abcde", 5, 0, 0);
}

void publish_cb(void *self, void *params)
{

}
void data_cb(void *self, void *params)
{
    mqtt_client *client = (mqtt_client *)self;
    mqtt_event_data_t *event_data = (mqtt_event_data_t *)params;
    //cJSON *request_json; 
    if(event_data->data_offset == 0) {

        /*char *topic = malloc(event_data->topic_length + 1);
        memcpy(topic, event_data->topic, event_data->topic_length);
        topic[event_data->topic_length] = 0;
        ESP_LOGI(MQTT_TAG, "[APP] Publish topic: %s", topic);
        free(topic);*/
	/*char *data = malloc(event_data->data_length + 1);
	memcpy(data, event_data->data, event_data->data_length);
	data[event_data->data_length] = 0;
	cJSON *request_json = cJSON_Parse(data);
	char *item = cJSON_GetObjectItem(request_json, "items");
	cJSON_Delete(request_json);
	ESP_LOGI(MQTT_TAG,"[APP] Publish data : %s", item);
	free(data);*/
	//mqtt_publish(client,"devices/gateway1/pub","done",4,0,0);
    }
    //cJSON_Delete(request_json);
    char *data = malloc(event_data->data_length + 1);
    memcpy(data, event_data->data, event_data->data_length);
    data[event_data->data_length] = 0;
    char *items = "{\"items\": \"huy\"}";        
    cJSON *request_json = cJSON_Parse(items);
    char *item = cJSON_GetObjectItem(request_json, "items")->valuestring;
    cJSON_Delete(request_json);    
    ESP_LOGI(MQTT_TAG,"[APP] Publish data : %s", item);
    free(data);    
    ESP_LOGI(MQTT_TAG, "[APP] Publish data[%d/%d bytes]",
             event_data->data_length + event_data->data_offset,
             event_data->data_total_length);
    mqtt_publish(client,"devices/gateway1/pub","abcde",5,0,0);
    // data);

    // free(data);

}

mqtt_settings settings = {
    .host = "makipos-smarthome.ddns.net",
/*#if defined(CONFIG_MQTT_SECURITY_ON)
    .port = 8883, // encrypted
#else*/
    .port = 1883, // unencrypted
//#endif
    .client_id = "mqtt_client_id",
    .username = "gateway1",
    .password = "mkpsmarthome",
    .clean_session = 0,
    .keepalive = 120,
    //.lwt_topic = "/lwt",
    //.lwt_msg = "offline",
    //.lwt_qos = 0,
    //.lwt_retain = 0,
    .connected_cb = connected_cb,
    //.disconnected_cb = disconnected_cb,
    //.reconnect_cb = reconnect_cb,
    .subscribe_cb = subscribe_cb,
    .publish_cb = publish_cb,
    .data_cb = data_cb
};

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            mqtt_start(&settings);
            //init app here
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            /* This is a workaround as ESP32 WiFi libs don't currently
               auto-reassociate. */
            esp_wifi_connect();
            mqtt_stop();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void wifi_conn_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_LOGI(MQTT_TAG, "start the WIFI SSID:[%s] password:[%s]", CONFIG_WIFI_SSID, "******");
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main()
{
    ESP_LOGI(MQTT_TAG, "[APP] Startup..");
    ESP_LOGI(MQTT_TAG, "[APP] Free memory: %d bytes", system_get_free_heap_size());
    ESP_LOGI(MQTT_TAG, "[APP] SDK version: %s, Build time: %s", system_get_sdk_version(), BUID_TIME);
    /*char *items = "{\"items\": 123}";
    cJSON *response_json = cJSON_Parse(items);
    double item = cJSON_GetObjectItem(response_json, "items")->valuedouble;
    ESP_LOGI(MQTT_TAG, "[APP] json %d", item);
    cJSON_Delete(response_json);*/    
    nvs_flash_init();
    wifi_conn_init();
}

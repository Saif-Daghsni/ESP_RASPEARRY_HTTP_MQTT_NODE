#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "dht.h"
#include "mqtt_client.h"
#include "esp_http_client.h"

/* ========================= Configuration ========================= */
// ===================== MQTT Configuration =====================
#define MQTT_BROKER_URI "mqtt://10.59.168.207:1883"  // FIXED: Match Node-RED broker
#define MQTT_TOPIC_TEMP "saif/temp"
#define MQTT_TOPIC_HUM  "saif/hum"
#define MQTT_TOPIC_LED   "topic/led"

// ===================== WiFi Configuration =====================
#define WIFI_SSID        "Saif Eddine's Galaxy A71"
#define WIFI_PASS        "saif123456"

// ===================== ThingSpeak Configuration =====================
#define TS_WRITE_API_KEY "562VM5FVYCRE0RO2"

// ===================== GPIO Configuration =====================
#define DHT_GPIO         5
#define LED_GPIO         2
#define POST_PERIOD_S    5  // Reduced to 5 seconds for better updates

static const char *TAG = "MQTT_DHT_ThingSpeak";

static EventGroupHandle_t s_wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;

static esp_mqtt_client_handle_t mqtt_client = NULL;

/* ========================= MQTT Event Handler ========================= */
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            // Subscribe to LED control topic
            esp_mqtt_client_subscribe(mqtt_client, MQTT_TOPIC_LED, 1);
            ESP_LOGI(TAG, "Subscribed to LED topic: %s", MQTT_TOPIC_LED);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT disconnected");
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Message received on %.*s: %.*s",
                     event->topic_len, event->topic,
                     event->data_len, event->data);

            // LED control via MQTT
            if (strncmp(event->topic, MQTT_TOPIC_LED, event->topic_len) == 0) {
                if (event->data_len > 0) {
                    char payload[16];
                    int len = event->data_len < sizeof(payload) - 1 ? event->data_len : sizeof(payload) - 1;
                    strncpy(payload, event->data, len);
                    payload[len] = '\0';
                    
                    ESP_LOGI(TAG, "LED command: %s", payload);
                    
                    // Handle various command formats
                    if (strcmp(payload, "1") == 0 || strcmp(payload, "ON") == 0 || 
                        strcmp(payload, "on") == 0 || strcmp(payload, "true") == 0) {
                        gpio_set_level(LED_GPIO, 1);
                        ESP_LOGI(TAG, "LED turned ON");
                    } else if (strcmp(payload, "0") == 0 || strcmp(payload, "OFF") == 0 || 
                               strcmp(payload, "off") == 0 || strcmp(payload, "false") == 0) {
                        gpio_set_level(LED_GPIO, 0);
                        ESP_LOGI(TAG, "LED turned OFF");
                    }
                }
            }
            break;

        default:
            break;
    }
}

/* ========================= MQTT Init ========================= */
static void mqtt_init(void)
{
    esp_mqtt_client_config_t config = {
        .broker.address.uri = MQTT_BROKER_URI,
        .session.keepalive = 60,
    };

    mqtt_client = esp_mqtt_client_init(&config);

    esp_mqtt_client_register_event(
        mqtt_client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL
    );

    esp_mqtt_client_start(mqtt_client);
}

/* ========================= WiFi Event Handler ========================= */
static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi disconnected, reconnecting...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* ========================= WiFi Init ========================= */
static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID,
        &wifi_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to WiFi: %s ...", WIFI_SSID);

    xEventGroupWaitBits(s_wifi_event_group,
                        WIFI_CONNECTED_BIT,
                        pdFALSE, pdFALSE,
                        portMAX_DELAY);
}

/* ========================= DHT11 Reading ========================= */
static bool dht11_read_values(float *temp, float *hum)
{
    float t = 0, h = 0;
    
    // Try reading twice
    for (int i = 0; i < 2; i++) {
        esp_err_t res = dht_read_float_data(DHT_TYPE_DHT11, DHT_GPIO, &h, &t);
        
        if (res == ESP_OK) {
            // Validate readings
            if (t >= -20.0 && t <= 80.0 && h >= 0.0 && h <= 100.0) {
                *temp = t;
                *hum = h;
                return true;
            }
        }
        
        if (i == 0) {
            ESP_LOGW(TAG, "DHT read attempt %d failed: %s", i+1, esp_err_to_name(res));
            vTaskDelay(pdMS_TO_TICKS(100));  // Short delay before retry
        }
    }
    
    ESP_LOGW(TAG, "DHT read failed after retries");
    return false;
}

/* ===================== THINGSPEAK HTTP POST ====================== */
static bool thingspeak_post(float temp, float hum)
{
    char post_data[128];

    snprintf(post_data, sizeof(post_data),
             "api_key=%s&field1=%.1f&field2=%.1f",
             TS_WRITE_API_KEY, temp, hum);

    esp_http_client_config_t config = {
        .url = "http://api.thingspeak.com/update",
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "ThingSpeak response HTTP %d", status);
        esp_http_client_cleanup(client);
        return (status == 200);
    } else {
        ESP_LOGE(TAG, "HTTP error: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }
}

/* ========================= MAIN ========================= */
void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Initialize LED GPIO
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0);  // Start with LED OFF
    
    ESP_LOGI(TAG, "LED initialized on GPIO %d", LED_GPIO);

    // Initialize WiFi
    wifi_init_sta();
    
    // Wait a moment for network stability
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Initialize MQTT
    mqtt_init();

    // Wait for MQTT connection
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "System initialized. Starting sensor readings...");

    while (1) {
        float t = 0, h = 0;

        if (dht11_read_values(&t, &h)) {
            ESP_LOGI(TAG, "Temperature = %.1f°C | Humidity = %.1f%%", t, h);

            // ============= MQTT PUBLISHING =============
            // Convert temperature to string
            char temp_str[16];
            snprintf(temp_str, sizeof(temp_str), "%.1f", t);

            // Convert humidity to string
            char hum_str[16];
            snprintf(hum_str, sizeof(hum_str), "%.1f", h);

            // Publish temperature with retain flag
            esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_TEMP, temp_str, 0, 1, 1);
            ESP_LOGI(TAG, "MQTT published TEMP to %s : %s", MQTT_TOPIC_TEMP, temp_str);

            // Publish humidity with retain flag
            esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_HUM, hum_str, 0, 1, 1);
            ESP_LOGI(TAG, "MQTT published HUM to %s : %s", MQTT_TOPIC_HUM, hum_str);

            // ============= THINGSPEAK UPLOAD =============
            bool ts_ok = thingspeak_post(t, h);
            if (ts_ok) {
                ESP_LOGI(TAG, "ThingSpeak update OK");
            } else {
                ESP_LOGW(TAG, "ThingSpeak update FAILED");
            }
            
        } else {
            ESP_LOGE(TAG, "Failed to read DHT sensor");
        }

        vTaskDelay(pdMS_TO_TICKS(POST_PERIOD_S * 1000));
    }
}

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "driver/twai.h"



#define PORT                        CONFIG_EXAMPLE_PORT
#define KEEPALIVE_IDLE              CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_EXAMPLE_KEEPALIVE_COUNT

#define MESSAGE_ID 0xA0
#define TWAI_RECEIVE_TASK_PRIO 9
#define TCP_TASK_PRIO 10
#define CTRL_TASK_PRIO 11

#define TX_GPIO_NUM 21
#define RX_GPIO_NUM 22

#define TCP_TAG "SENDING"
#define READ_TAG "READING"
#define GEN_TAG "GENERAL"


// CAN ID = 4 bytes, CAN DATA = 8 bytes, CAN DLC = 1 byte
const int tcp_payload_size = 13;

// CAN frequency 500k bits 
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

static twai_message_t data_message = {
    // Message type and format settings
    .extd = 0,              // Standard Format message (11-bit ID)
    .rtr = 0,               // Send a data frame
    .ss = 0,                // Not single shot
    .self = 0,              // Not a self reception request
    .dlc_non_comp = 0,      // DLC is less than 8
    // Message ID and payload
    .identifier = MESSAGE_ID,
    .data_length_code = 0,
    .data = {0},
};

static twai_message_t tcp_message;

// Syncrhonization
static SemaphoreHandle_t ctrl_task_sem;
static SemaphoreHandle_t twai_receive_sem;
static SemaphoreHandle_t done_sem;
static QueueHandle_t tcp_task_queue;


static void twai_receive_task(void *arg)
{
    while (true)
    {
        xSemaphoreTake(twai_receive_sem, portMAX_DELAY);
        ESP_LOGI(READ_TAG, "Receiving data");
        twai_receive(&data_message, portMAX_DELAY);
        ESP_LOGI(READ_TAG, "Received data with %" PRIu32 " ", data_message.identifier);
        ESP_LOGI(READ_TAG, "Data: ");
        for (int i = 0; i < 8; ++i)
        {
            ESP_LOGI(READ_TAG, "%u", data_message.data[i]);
        }

        xSemaphoreGive(ctrl_task_sem);  
        xQueueSend(tcp_task_queue, &data_message, portMAX_DELAY);
    }

    vTaskDelete(NULL);
}

// Sends CAN message via TCP
static int tcp_transmit(const int sock)
{

    int len = tcp_payload_size;
    char rx_buffer[len + 10];
    int offset = 0;
    memcpy(rx_buffer, &(tcp_message.identifier), sizeof(tcp_message.identifier));
    offset += sizeof(tcp_message.identifier);
    memcpy((rx_buffer + offset), &(tcp_message.data_length_code), sizeof(tcp_message.data_length_code));
    offset += sizeof(tcp_message.data_length_code);
    memcpy((rx_buffer + offset), &(tcp_message.data), sizeof(tcp_message.data));
    
        
    if (len < 0) {
            ESP_LOGE(TAG, "Error occurred: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string

            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation.
            // Send CAN message via TCP
            int to_write = len;
            while (to_write > 0) {
                int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
                if (written < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    // Failed to transmit , giving up
                    return -1;
                }
                to_write -= written;
            }
        }

    return 0;
}

static const char* SCAN_TAG = "SCAN";

static void scan_and_print(void)
{
    wifi_scan_config_t scan_cfg = {
        .ssid = 0,
        .bssid = 0,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 100,
        .scan_time.active.max = 300,
    };

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_cfg, true));

    uint16_t ap_num = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_num));
    wifi_ap_record_t *recs = calloc(ap_num, sizeof(wifi_ap_record_t));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, recs));

    ESP_LOGI(SCAN_TAG, "Found %u APs", ap_num);
    for (int i = 0; i < ap_num; i++) {
        ESP_LOGI(SCAN_TAG, "SSID: '%s' RSSI:%d chan:%d auth:%d",
                 (char*)recs[i].ssid, recs[i].rssi, recs[i].primary, recs[i].authmode);
    }
    free(recs);
}

static void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    

#ifdef CONFIG_EXAMPLE_IPV4
    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }
#endif


    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {

        ESP_LOGI(TAG, "Socket ready to send");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
#ifdef CONFIG_EXAMPLE_IPV4
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
#endif

        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        while (1)
        {
            if (xQueueReceive(tcp_task_queue, &tcp_message, portMAX_DELAY) == pdTRUE)
            {
                ESP_LOGI(TCP_TAG, "Sending can frame over TCP");
                if (tcp_transmit(sock) < 0)
                    break;
            }
        }
       
        
        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

static void control_task(void *arg)
{
    while (true)
    {
        xSemaphoreTake(ctrl_task_sem, portMAX_DELAY);
        // Read data from CAN bus
        xSemaphoreGive(twai_receive_sem);

    }
}

void app_main(void)
{
   
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
//     esp_netif_create_default_wifi_sta();

// wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
// ESP_ERROR_CHECK(esp_wifi_init(&cfg));
// ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
// ESP_ERROR_CHECK(esp_wifi_start());   // <-- THIS is why you were failing


//      scan_and_print();

    ESP_ERROR_CHECK(example_connect());

    wifi_scan_config_t scan_cfg = {
    .ssid = 0,
    .bssid = 0,
    .channel = 0,
    .show_hidden = true
};
ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_cfg, true));

    // Syncrhonization
    ctrl_task_sem = xSemaphoreCreateBinary();
    twai_receive_sem = xSemaphoreCreateBinary();
    done_sem = xSemaphoreCreateBinary();
    tcp_task_queue = xQueueCreate(5, sizeof(twai_message_t));
    
    // Create tasks
    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_Receive", 4096, NULL, TWAI_RECEIVE_TASK_PRIO, NULL, 1);
    xTaskCreatePinnedToCore(control_task, "TWAI_ctrl", 4096, NULL, CTRL_TASK_PRIO, NULL, 1);
#ifdef CONFIG_EXAMPLE_IPV4
    xTaskCreatePinnedToCore(tcp_server_task, "tcp_task", 4096, (void*)AF_INET, TCP_TASK_PRIO, NULL, 0);
#endif


     //Install TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(GEN_TAG, "Driver installed");


    // Start the TWAI driver
    ESP_ERROR_CHECK(twai_start());  
    ESP_LOGI(GEN_TAG, "TWAI started");

    // Start program
    xSemaphoreGive(ctrl_task_sem);
    xSemaphoreTake(done_sem, portMAX_DELAY);   // Wait for completion

    ESP_ERROR_CHECK(twai_stop());  
    ESP_LOGI(GEN_TAG, "TWAI stopped");


    //Uninstall TWAI driver
    ESP_ERROR_CHECK(twai_driver_uninstall());
    ESP_LOGI(GEN_TAG, "Driver uninstalled");

    // Cleanup
    vSemaphoreDelete(ctrl_task_sem);
    vSemaphoreDelete(twai_receive_sem);
    vQueueDelete(tcp_task_queue);

}

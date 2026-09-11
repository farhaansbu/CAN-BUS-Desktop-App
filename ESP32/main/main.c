#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>

#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "lwip/inet.h"

#include "driver/twai.h"


static const char *TAG = "WIFI_VEHICLE_AP";

/* ------- Wi-Fi SoftAP ------- */
#define AP_SSID                 "ESP32_VEHICLE"
#define AP_PASS                 "VehicleTest2026"
#define AP_CHANNEL              6
#define AP_MAX_CONN             4


/* ----- Wi-Fi Connection ----- */
/*
#define PORT                        CONFIG_EXAMPLE_PORT
#define KEEPALIVE_IDLE              CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_EXAMPLE_KEEPALIVE_COUNT
*/ 
/* ----- Wi-Fi Connection ----- */

#define SERVER_IP "192.168.4.2" // update as needed for the PC
#define PORT                3333

#define KEEPALIVE_IDLE      5
#define KEEPALIVE_INTERVAL  5
#define KEEPALIVE_COUNT     3


// -- CAN Bus --

#define MESSAGE_ID 0xA0
#define FUNCTIONAL_REQUEST_ID 0x7DF

#define TWAI_RECEIVE_TASK_PRIO 8
#define TWAI_TRANSMIT_TASK_PRIO 9
#define TCP_TASK_PRIO 10
#define CTRL_TASK_PRIO 11

#define TX_GPIO_NUM 21
#define RX_GPIO_NUM 22

#define TCP_TAG "SENDING"
#define READ_TAG "READING"
#define GEN_TAG "GENERAL"

#define SERVICE_MODE_1 1

// CAN ID = 4 bytes, CAN DATA = 8 bytes, CAN DLC = 1 byte
const int tcp_payload_size = 13;

// CAN frequency 500k bits 
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
/* static const twai_filter_config_t f_config = 
{ 
    .acceptance_code = 0x7E8 << 21,
    .acceptance_mask = (0x007 << 21) | 0x1FFFFF,
    .single_filter = true,
};*/

/* 0x7E8 (0b 0111 1110 1000) to 0x7EF (0b 0111 1110 1111)
    The least three significant bits are the only ones 
    that don't matter in the 11-bit ID

    Legacy ESP-IDF: don't care about 1s in the filter
    0b 000 0000 0111 (0x007)
    We shift this 11-bit pattern to the left of the 32 bit mask 
    (meaning 21 bits shifted left)

    the 21 bits on the right now are don't care's 
    so we extend 1s to the right 
    0b 1 1111 1111 1111 1111 1111 (0x1FFFFF)
*/


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
static SemaphoreHandle_t twai_transmit_sem;
static SemaphoreHandle_t done_sem;
static QueueHandle_t tcp_task_queue;

// PIDs
#define NUM_PID_TABLE_ELEM 6 // update as needed

#define PID_SUPPORTED 0x00 // only called once at the beginning

typedef enum {
    ENGINE_COOLANT_TEMP = 0x05,
    ENGINE_SPEED = 0x0C, 
    VEHICLE_SPEED = 0x0D,
    INTAKE_AIR_TEMP = 0x0F,
    THROTTLE_POSITION = 0x11,
    ENGINE_RUN_TIME = 0x1F,

} PID_VALUES;

static const PID_VALUES table_PIDs[NUM_PID_TABLE_ELEM] = {
    ENGINE_COOLANT_TEMP, ENGINE_SPEED, VEHICLE_SPEED, 
    INTAKE_AIR_TEMP, THROTTLE_POSITION, ENGINE_RUN_TIME
};

// ----- TASKS and HELPER FUNCTIONS -----

// CAN/TWAI Receive Task
static void twai_receive_task(void *arg)
{
    while (true)
    {
        xSemaphoreTake(twai_receive_sem, portMAX_DELAY);
        ESP_LOGI(READ_TAG, "Receiving data");
        
        if ( (twai_receive(&data_message, portMAX_DELAY) ) == ESP_OK) {
            ESP_LOGI(READ_TAG, "Received data with %" PRIu32 " ", data_message.identifier);
            ESP_LOGI(READ_TAG, "Data: ");
            for (uint8_t i = 0; i < 8; ++i) {
                ESP_LOGI(READ_TAG, "%u", data_message.data[i]);
            }
            xQueueSend(tcp_task_queue, &data_message, portMAX_DELAY);
        }

        xSemaphoreGive(ctrl_task_sem);  
    }

    vTaskDelete(NULL);
}


// Helper function for confirming that PIDs are supported
void query_PID(twai_message_t* pid_query_request_ptr, uint8_t PID_value) {
    
    ESP_LOGI(TCP_TAG, "Sending query request");
    
    pid_query_request_ptr->data[0] = 2;
    pid_query_request_ptr->data[1] = SERVICE_MODE_1;
    pid_query_request_ptr->data[2] = PID_value;
    for (uint8_t i=3; i<8; i++) {
        pid_query_request_ptr->data[i] = 0xCC;
    }

    // is this the correct way of using error check?
    // and how long should we wait in case the buffer is full?
    //ESP_ERROR_CHECK(twai_transmit(&pid_query_request, portMAX_DELAY));
    //if (twai_transmit(&pid_query_request, portMAX_DELAY) == ESP_OK) {
    if (twai_transmit(pid_query_request_ptr, portMAX_DELAY) == ESP_OK) {
        // print out the PID as well
        ESP_LOGI(TCP_TAG, "Transmitted query for service mode 01 PID %" PRIu8 " ", PID_value);
    } 
}

// CAN/TWAI Transmit Task
static void twai_transmit_task(void *arg)
{
    xSemaphoreTake(twai_transmit_sem, portMAX_DELAY);

    twai_message_t pid_query_request = {
        // Message type and format settings
        .extd = 0,              // Standard Format message (11-bit ID)
        .rtr = 0,               // Send a data frame
        .ss = 0,                // Not single shot
        .self = 0,              // Not a self reception request
        .dlc_non_comp = 0,      // DLC is less than 8
        // Message ID and payload
        .identifier = FUNCTIONAL_REQUEST_ID,
        .data_length_code = 8,
        .data = {0},
    };
    
    uint8_t pid_mode_1;

    // first confirm that the PIDs are supported
    query_PID(&pid_query_request, PID_SUPPORTED);

    while (true) {

        for (uint8_t i = 0; i < NUM_PID_TABLE_ELEM; i++) {
            
            pid_mode_1 = table_PIDs[i];

            // call helper function to transmit CAN IDs
            query_PID(&pid_query_request, pid_mode_1);
        }

        //Change the delay here as needed
        vTaskDelay(pdMS_TO_TICKS(10));
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
            //rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string

            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation.
            // Send CAN message via TCP
            int to_write = len;
            while (to_write > 0) {
                int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
                if (written <= 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    // Failed to transmit , giving up
                    return -1;
                }
                to_write -= written;
            }
        }

    return 0;
}


/* Wi-Fi SoftAP initialization */
static void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = { 0 };

    memcpy(wifi_config.ap.ssid, AP_SSID, strlen(AP_SSID));
    memcpy(wifi_config.ap.password, AP_PASS, strlen(AP_PASS));
    wifi_config.ap.ssid_len = strlen(AP_SSID);
    wifi_config.ap.channel = AP_CHANNEL;
    wifi_config.ap.max_connection = AP_MAX_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "ESP32 SoftAP started");
    ESP_LOGI(TAG, "SSID: %s", AP_SSID);
    ESP_LOGI(TAG, "Password: %s", AP_PASS);
    //ESP_LOGI(TAG, "AP IP: 192.168.4.1");
    ESP_LOGI(TAG, "TCP client target: %s:%d", SERVER_IP, PORT);
}

// Establishes TCP connection to the C++ desktop server
// and sends received CAN frames to it
static void tcp_client_task(void *pvParameters)
{
    (void)pvParameters;

    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;

    while (1)
    {
        struct sockaddr_in dest_addr = {0};

        // Destination = PC running the C++ TCP server
        dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);

        // Create TCP socket
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(3000));
            continue;
        }

        // Configure TCP keepalive
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

        ESP_LOGI(TAG, "Connecting to C++ TCP server at %s:%d", SERVER_IP, PORT);

        // ESP32 initiates connection to PC
        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

        if (err != 0)
        {
            ESP_LOGW(TAG, "Unable to connect to TCP server: errno %d", errno);
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(3000));
            continue;
        }

        ESP_LOGI(TAG, "Connected to C++ TCP server at %s:%d", SERVER_IP, PORT);

        // Once connected, wait for CAN frames
        while (1)
        {
            if (xQueueReceive(tcp_task_queue, &tcp_message, portMAX_DELAY) == pdTRUE)
            {
                ESP_LOGI(TCP_TAG, "Sending CAN frame over TCP");

                if (tcp_transmit(sock) < 0)
                {
                    ESP_LOGW(TAG, "TCP connection lost");
                    break;
                }
            }
        }

        // Close dead connection
        shutdown(sock, SHUT_RDWR);
        close(sock);

        ESP_LOGW(TAG, "Socket closed, retrying connection...");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    vTaskDelete(NULL);
}

/* primary task for synchronization between 
   receiving data and Wi-Fi */
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

    wifi_init_softap();
    
    // Syncrhonization
    ctrl_task_sem = xSemaphoreCreateBinary();
    twai_receive_sem = xSemaphoreCreateBinary();
    twai_transmit_sem = xSemaphoreCreateBinary();
    done_sem = xSemaphoreCreateBinary();
    tcp_task_queue = xQueueCreate(5, sizeof(twai_message_t));
    
    // Create tasks
    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_Receive", 4096, NULL, TWAI_RECEIVE_TASK_PRIO, NULL, 1);
    xTaskCreatePinnedToCore(control_task, "TWAI_ctrl", 4096, NULL, CTRL_TASK_PRIO, NULL, 1);
    xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_TRANSMIT", 4096, NULL, TWAI_TRANSMIT_TASK_PRIO, NULL, 1);
    xTaskCreatePinnedToCore(tcp_client_task, "tcp_task", 4096, NULL, TCP_TASK_PRIO, NULL, 0);

     //Install TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(GEN_TAG, "Driver installed");

    // Start the TWAI driver
    ESP_ERROR_CHECK(twai_start());  
    ESP_LOGI(GEN_TAG, "TWAI started");

    // Start program
    xSemaphoreGive(ctrl_task_sem);
    xSemaphoreGive(twai_transmit_sem);
    xSemaphoreTake(done_sem, portMAX_DELAY);   // Wait for completion

    // Stop the TWAI driver
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
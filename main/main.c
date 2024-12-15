/**
 * @file main.c
 * @brief UART communication with data storage in NVS.
 * 
 * This program initializes UART communication on an ESP32 device
 * with specific configurations. Data received via UART is stored in
 * NVS (Non-Volatile Storage) and then transmitted back. The program
 * also calculates transmission and reception speeds.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"

/** Handle for NVS storage */
nvs_handle_t my_handle;

/** Size of UART receive buffer */
static const int RX_BUF_SIZE = 1024;

/** UART pin configurations */
#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

/**
 * @brief Initialize UART with specific configuration.
 * 
 * Sets up the UART driver, including parameters such as baud rate,
 * data bits, stop bits, and flow control.
 */
void uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 2400,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // Install UART driver with RX buffer and no TX buffer
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

/**
 * @brief Send data over UART and log transmission speed.
 * 
 * @param[in] logName Name of the log tag
 * @param[in] data Data string to transmit
 */
void sendData(const char* logName, const char* data) {
    const int len = strlen(data);
    TickType_t start_time = xTaskGetTickCount();
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    TickType_t end_time = xTaskGetTickCount();

    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    float time_taken = end_time - start_time;
    float speed = (txBytes / time_taken) * configTICK_RATE_HZ; // bytes per second
    ESP_LOGI(logName, "Transmission Speed: %.2f bytes/sec", speed);

    // Erase all entries and commit the changes in NVS
    nvs_erase_all(my_handle);
    nvs_commit(my_handle);
}

/**
 * @brief Task to handle UART data transmission.
 * 
 * Reads data stored in NVS, transmits it over UART, and calculates the
 * transmission speed.
 */
void tx_task(void) {
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);

    char *data = (char *)malloc(RX_BUF_SIZE);
    size_t data_size = RX_BUF_SIZE;
    
    // Read data from NVS and send it over UART
    nvs_get_str(my_handle, "uart", (char*)data, &data_size);
    sendData(TX_TASK_TAG, data);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    free(data);
}

/**
 * @brief Task to handle UART data reception.
 * 
 * Waits for incoming UART data, stores it in NVS, and calculates
 * the reception speed. Once received, it triggers the transmission task.
 * 
 * @param[in] arg Task input argument (unused)
 */
static void rx_task(void *arg) {
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);

    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE + 1);
    while (1) {
        ESP_LOGI(RX_TASK_TAG, "WAITING FOR DATA");

        // Measure reception time and read UART data
        TickType_t start_time = xTaskGetTickCount();
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        TickType_t end_time = xTaskGetTickCount();

        if (rxBytes > 0) {
            data[rxBytes] = 0; // Null-terminate received data

            // Store data into NVS
            nvs_set_str(my_handle, "uart", (const char*)data);
            nvs_commit(my_handle);

            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            float time_taken = end_time - start_time;
            float speed = (rxBytes / time_taken) * configTICK_RATE_HZ; // bytes per second
            ESP_LOGI(RX_TASK_TAG, "Reception Speed: %.2f bytes/sec", speed);

            // Trigger transmission task
            tx_task();
        }
        
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
    free(data);
}

/**
 * @brief Application entry point.
 * 
 * Initializes UART and NVS, and creates the UART receive task.
 */
void app_main(void) {
    // Initialize UART and NVS
    uart_init();
    nvs_flash_init();
    nvs_open("storage", NVS_READWRITE, &my_handle);

    // Create UART receive task
    xTaskCreate(rx_task, "uart_rx_task", 1024 * 3, NULL, configMAX_PRIORITIES, NULL);
}

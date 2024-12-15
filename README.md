# UART Communication with NVS Data Storage on ESP32

This project demonstrates UART (Universal Asynchronous Receiver-Transmitter) communication on the ESP32 platform. Data received over UART is stored in **Non-Volatile Storage (NVS)**, a replacement for EEPROM on ESP32, and subsequently transmitted back with calculated communication speeds.

---

## Table of Contents
1. [Overview](#overview)
2. [Hardware Requirements](#hardware-requirements)
3. [Key Features](#key-features)
4. [Project Description](#project-description)
5. [Code Walkthrough](#code-walkthrough)
6. [Building and Flashing](#building-and-flashing)
7. [Important Notes](#important-notes)
8. [References](#references)

---

## Overview
This program implements:
- UART initialization with user-defined configurations.
- Reception of data over UART and its storage in **NVS**.
- Transmission of received data back over UART with speed calculations for both RX and TX.
- Usage of **FreeRTOS tasks** to manage reception and transmission concurrently.

ESP32 does not have built-in EEPROM like some other microcontrollers. Instead, the **NVS (Non-Volatile Storage)** API is used to store data persistently.

---

## Hardware Requirements
1. **ESP32 Development Board** (e.g., ESP32-DevKitC)
2. USB Cable for flashing and serial monitoring
3. UART Communication Setup:
   - TX Pin: GPIO 4
   - RX Pin: GPIO 5

4. Optional:
   - External UART module (e.g., USB-to-TTL converter) for testing

---

## Key Features
- **UART Communication**: Bidirectional communication with configurable baud rate and buffer sizes.
- **Persistent Data Storage**: Data is saved in NVS, ensuring persistence across resets.
- **Speed Measurement**: Measures transmission and reception speeds in bytes/second.
- **FreeRTOS Tasks**: Uses tasks for handling RX and TX processes efficiently.

---

## Project Description
The program performs the following tasks:

1. **Initialization**:
   - Configures UART1 with the following settings:
     - Baud Rate: **2400 bps**
     - Data Bits: **8 bits**
     - Stop Bits: **1 bit**
     - No Parity or Flow Control
   - Initializes the Non-Volatile Storage (NVS).

2. **Reception Task**:
   - Waits for incoming data on the UART interface.
   - Calculates the reception speed (bytes per second).
   - Stores the received data into NVS.
   - Triggers the transmission task.

3. **Transmission Task**:
   - Reads the stored data from NVS.
   - Transmits the data over UART.
   - Calculates and logs the transmission speed.

4. **Speed Calculation**:
   - Reception and transmission speeds are calculated using FreeRTOS tick counts.

---

## Code Walkthrough
### 1. **UART Initialization**
The `init()` function configures UART1 with the following parameters:
```c
const uart_config_t uart_config = {
    .baud_rate = 2400,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
};
```
- RX Buffer size: **1024 bytes**
- TX Buffer: Not used

Pins:
- **TXD_PIN**: GPIO 4
- **RXD_PIN**: GPIO 5

---

### 2. **Reception Task (RX)**
The `rx_task` handles incoming data:
- Waits for data on UART1 for up to **1 second**.
- Measures the reception speed.
- Saves the received data into NVS using:
  ```c
  nvs_set_str(my_handle, "uart", (const char*)data);
  nvs_commit(my_handle);
  ```
- Once data is stored, it triggers the transmission task.

---

### 3. **Transmission Task (TX)**
The `tx_task` reads data from NVS and sends it back over UART:
- Reads stored data using `nvs_get_str()`.
- Transmits the data over UART.
- Measures and logs the transmission speed.

**Data Erasure:**
After transmission, all entries in NVS are erased to ensure a clean state:
```c
nvs_erase_all(my_handle);
nvs_commit(my_handle);
```

---

### 4. **Main Function**
The `app_main()` function initializes UART and NVS, then starts the reception task:
```c
init();
nvs_flash_init();
nvs_open("storage", NVS_READWRITE, &my_handle);
xTaskCreate(rx_task, "uart_rx_task", 1024 * 3, NULL, configMAX_PRIORITIES, NULL);
```

---

## Building and Flashing
1. **Prerequisites**:
   - Install [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/) development environment.
   - Connect the ESP32 to your development machine via USB.

2. **Build and Flash**:
   ```bash
   idf.py build
   idf.py flash
   ```

3. **Monitor Output**:
   ```bash
   idf.py monitor
   ```
   - Use the monitor to observe UART logs, including received data, transmission/reception speeds, and task activity.

---

## Important Notes
1. **NVS Instead of EEPROM**:
   - Unlike Arduino boards, the ESP32 does not have a dedicated EEPROM.
   - The NVS library is used for persistent storage, allowing you to read/write data to flash memory.
   - Ensure NVS is initialized properly with `nvs_flash_init()`.

2. **UART Pin Configuration**:
   - Default GPIOs:
     - **TX Pin**: GPIO 4
     - **RX Pin**: GPIO 5
   - Adjust these pins in the `#define` section if needed.

3. **Buffer Management**:
   - Ensure sufficient memory allocation for data buffers.
   - RX and TX tasks use a buffer size of **1024 bytes**.

4. **Baud Rate**:
   - The program currently uses **2400 bps**. Adjust as required for your application.

5. **Speed Calculation**:
   - Speed is calculated using FreeRTOS tick counts, assuming the system tick rate defined in `configTICK_RATE_HZ`.

---

## References
1. **ESP-IDF Documentation**:
   - UART API: [UART Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)
   - NVS API: [Non-Volatile Storage](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html)

2. **FreeRTOS**:
   - Task Management: [FreeRTOS API](https://www.freertos.org/a00106.html)

---

## Author
- **Prathamesh Wanjale**
- Embedded Firmware Engineer

---

Enjoy implementing UART communication and NVS storage with ESP32! ✅


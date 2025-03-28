#include "serial_comm.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void serial_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, 
                UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
}

void send_test_data(void) {
    uint8_t test_data[256];
    for (int i = 0; i < sizeof(test_data); i++) {
        test_data[i] = i;
    }
    
    uint8_t header[4] = {0xAA, 0x55, (sizeof(test_data) >> 8) & 0xFF, 
                        sizeof(test_data) & 0xFF};
    uart_write_bytes(UART_NUM, (const char*)header, sizeof(header));
    uart_write_bytes(UART_NUM, (const char*)test_data, sizeof(test_data));
    printf("Sent test data frame\n");
}

void send_camera_data(const uint8_t *data, size_t len) {
    uint8_t header[4] = {0xAA, 0x55, (len >> 8) & 0xFF, len & 0xFF};
    uart_write_bytes(UART_NUM, (const char*)header, sizeof(header));
    uart_write_bytes(UART_NUM, (const char*)data, len);
}
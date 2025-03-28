#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include "driver/uart.h"

#define UART_NUM UART_NUM_0
#define BUF_SIZE (1024)

void serial_init(void);
void send_test_data(void);
void send_camera_data(const uint8_t *data, size_t len);

#endif
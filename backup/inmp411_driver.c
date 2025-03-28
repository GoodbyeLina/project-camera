#include "inmp411_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

// 配置参数
#define I2S_SAMPLE_RATE     16000
#define I2S_DATA_PIN        12
#define I2S_BCK_PIN         13
#define I2S_WS_PIN          14
#define I2S_BUFFER_SIZE     1024

static i2s_chan_handle_t rx_handle;
static char save_path[256] = {0};
static int file_fd = -1;

static void record_task(void *arg) {
    int16_t *buffer = malloc(I2S_BUFFER_SIZE * sizeof(int16_t));
    size_t bytes_read;
    bool save_to_pc = (bool)arg;
    
    if (save_to_pc && save_path[0] != '\0') {
        file_fd = open(save_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (file_fd < 0) {
            ESP_LOGE("RECORD", "Failed to open file: %s", save_path);
            vTaskDelete(NULL);
            return;
        }
    }
    
    while(1) {
        i2s_channel_read(rx_handle, buffer, I2S_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
        printf("Recorded %d bytes\n", bytes_read);
        
        if (save_to_pc && file_fd >= 0) {
            write(file_fd, buffer, bytes_read);
        }
        
        vTaskDelay(10);
    }
    
    if (file_fd >= 0) {
        close(file_fd);
        file_fd = -1;
    }
}

void inmp411_init() {
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, NULL, &rx_handle);

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT, 
            I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_PIN,
            .ws = I2S_WS_PIN,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_DATA_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    
    i2s_channel_init_std_mode(rx_handle, &std_cfg);
}

void inmp411_set_save_path(const char* path) {
    strncpy(save_path, path, sizeof(save_path)-1);
    save_path[sizeof(save_path)-1] = '\0';
}

void inmp411_start_record(bool save_to_pc) {
    i2s_channel_enable(rx_handle);
    xTaskCreate(record_task, "record_task", 4096, (void*)save_to_pc, 5, NULL);
}
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "inmp411_driver.h"
#include "ov7670_driver.h"
#include "serial_comm.h"

#define TEST_MODE 1  // 1=测试模式 0=正常摄像头模式

void app_main(void)
{
    // 初始化串口
    serial_init();

#if TEST_MODE
    while(1) {
        send_test_data();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
#else
    // 初始化麦克风
    inmp411_init();
    inmp411_set_save_path("C:/Users/97978/Desktop/recording.wav");
    inmp411_start_record(true);

    // 初始化摄像头
    if (my_ov7670_init() != ESP_OK) {
        printf("OV7670 init failed\n");
        return;
    }

    while (1) {
        // 捕获图像帧
        camera_fb_t *fb = my_ov7670_capture();
        if (!fb) {
            printf("Camera capture failed\n");
            continue;
        }

        printf("Captured frame: %dx%d, format: %d, size: %d\n",
               fb->width, fb->height, fb->format, fb->len);

        // 通过串口发送图像数据
        send_camera_data(fb->buf, fb->len);
        
        // 释放帧缓冲区
        my_ov7670_return_fb(fb);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

#endif
}
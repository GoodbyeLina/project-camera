# ESP32-S3 驱动OV7670摄像头模块技术文档

## 硬件连接
- OV7670 VCC → 3.3V
- OV7670 GND → GND
- OV7670 SIOC → ESP32-S3 I2C_SCL (GPIO_NUM_39)
- OV7670 SIOD → ESP32-S3 I2C_SDA (GPIO_NUM_38)
- OV7670 VSYNC → ESP32-S3 GPIO_NUM_48
- OV7670 HREF → ESP32-S3 GPIO_NUM_42
- OV7670 PCLK → ESP32-S3 GPIO_NUM_43
- OV7670 XCLK → ESP32-S3 GPIO_NUM_45
- OV7670 D0-D7 → ESP32-S3 GPIO_NUM_40-47

## 软件架构
1. 使用ESP-IDF官方摄像头驱动
2. 独立驱动文件：ov7670_driver.c/h
3. 主要功能：
   - 初始化摄像头
   - 配置图像参数(分辨率/格式)
   - 图像采集回调
   - 图像处理接口

## API设计
```c
// 初始化摄像头
esp_err_t my_ov7670_init();

// 获取图像帧
camera_fb_t* my_ov7670_capture();

// 释放帧缓冲区
void my_ov7670_return_fb(camera_fb_t* fb);
```

## 示例代码
```c
#include "ov7670_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main() {
    if (my_ov7670_init() != ESP_OK) {
        printf("Camera init failed\n");
        return;
    }

    while (1) {
        camera_fb_t *fb = my_ov7670_capture();
        if (!fb) {
            printf("Capture failed\n");
            continue;
        }

        printf("Frame: %dx%d, size: %d\n", fb->width, fb->height, fb->len);
        
        // 处理图像帧...
        
        my_ov7670_return_fb(fb);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
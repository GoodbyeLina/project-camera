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

### 管脚配置注意事项
1. 摄像头与麦克风共用GPIO_NUM_48(WS/VSYNC)
   - 通过分时复用解决冲突
   - 初始化时会自动配置
2. 其他管脚无冲突
3. 完整管脚分配表：

| 设备   | 信号线 | GPIO引脚 |
|--------|--------|----------|
| 摄像头 | VSYNC  | 48       |
| 摄像头 | HREF   | 42       |
| 摄像头 | PCLK   | 43       |
| 摄像头 | XCLK   | 45       |
| 摄像头 | D0-D7  | 40-47    |
| 麦克风 | DATA   | 21       |
| 麦克风 | BCK    | 47       |
| 麦克风 | WS     | 48       |

## 软件架构
1. 使用ESP-IDF官方摄像头驱动
2. 独立驱动文件：ov7670_driver.c/h
3. 主要功能：
   - 初始化摄像头
   - 配置图像参数(分辨率/格式)
   - 图像采集回调
   - 图像处理接口
4. 与麦克风协同工作：
   - 共享GPIO48通过分时复用
   - 初始化顺序：先摄像头后麦克风
   - 自动处理管脚冲突

## 初始化日志示例
```
I (1234) OV7670: 摄像头初始化成功
I (1234) OV7670: 配置参数：
I (1234) OV7670:   VSYNC引脚: 48
I (1234) OV7670:   HREF引脚: 42
I (1234) OV7670:   PCLK引脚: 43
I (1234) OV7670:   XCLK引脚: 45
I (1234) OV7670:   数据引脚: 40-47
I (1234) OV7670:   分辨率: 640x480
```

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
        ESP_LOGE("OV7670", "Camera init failed");
        return;
    }

    while (1) {
        camera_fb_t *fb = my_ov7670_capture();
        if (!fb) {
            ESP_LOGE("OV7670", "Capture failed");
            continue;
        }

        ESP_LOGI("OV7670", "Frame: %dx%d, size: %d", fb->width, fb->height, fb->len);
        
        // 处理图像帧...
        
        my_ov7670_return_fb(fb);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
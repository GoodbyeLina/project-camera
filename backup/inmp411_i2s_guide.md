# ESP32-S3 使用I2S驱动INMP411麦克风模块技术文档

## 硬件连接
- INMP411 VDD → 3.3V
- INMP411 GND → GND
- INMP411 OUT → ESP32-S3 I2S数据输入引脚
- INMP411 L/R → GND (单声道模式)
- INMP411 WS → ESP32-S3 I2S时钟引脚(可选)

推荐引脚配置：
- DATA: GPIO_NUM_12
- BCK: GPIO_NUM_13  
- WS: GPIO_NUM_14

## 软件实现要点

### 1. I2S配置
使用ESP-IDF提供的i2s_std_config_t结构体配置I2S:
- 采样率: 16kHz (INMP411支持8-16kHz)
- 数据位宽: 16bit
- 通道格式: I2S_CHANNEL_FMT_ONLY_LEFT
- 通信格式: I2S_COMM_FORMAT_STAND_I2S

### 2. 音频数据处理
- 使用DMA缓冲区接收音频数据
- 可添加环形缓冲区实现实时处理
- 支持三种输出方式:
  * 本地打印数据信息
  * 通过串口传输WAV格式数据(921600波特率)
  * 直接保存到电脑文件系统
- 串口传输说明:
  - 使用UART0(默认引脚)
  - 自动添加WAV文件头(16kHz, 16bit, 单声道)
  - 电脑端接收后可直接保存为.wav文件播放
- 文件保存说明:
  - 调用inmp411_set_save_path()设置保存路径
  - 调用inmp411_start_record(true)启用保存功能
  - 录音数据将实时写入指定.wav文件

### 3. 电源管理
- INMP411需要3.3V供电
- 注意ESP32-S3的I/O电压匹配

## API说明

### 文件保存相关API
- `void inmp411_set_save_path(const char* path)`
  - 功能：设置录音文件保存路径
  - 参数：path - 文件保存路径(支持绝对路径)
  - 示例：`inmp411_set_save_path("C:/Users/name/Desktop/recording.wav")`

- `void inmp411_start_record(bool save_to_pc)`
  - 功能：开始录音
  - 参数：save_to_pc - true表示启用文件保存功能
  - 注意：需先调用inmp411_set_save_path()设置路径

## 使用说明

1. 编译程序：
```bash
idf.py build
```

2. 烧录固件：
```bash
idf.py -p PORT flash
```
(将PORT替换为实际串口号，如COM3)

3. 运行程序：
- 程序启动后会自动开始录音
- 默认保存到设置的路径
- 按复位键可重新开始录音

4. 文件保存：
- 确保路径有写入权限
- Windows路径示例："C:/Users/name/Desktop/recording.wav"
- Linux路径示例："/home/user/recording.wav"

## 示例代码结构
```c
#include "driver/i2s.h"

// I2S配置
i2s_std_config_t i2s_config = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
        I2S_DATA_BIT_WIDTH_16BIT, 
        I2S_SLOT_MODE_MONO),
    .gpio_cfg = {
        .mclk = I2S_GPIO_UNUSED,
        .bclk = GPIO_NUM_13,
        .ws = GPIO_NUM_14,
        .dout = I2S_GPIO_UNUSED,
        .din = GPIO_NUM_12,
        .invert_flags = {
            .mclk_inv = false,
            .bclk_inv = false,
            .ws_inv = false,
        },
    },
};

void init_i2s() {
    i2s_chan_handle_t tx_handle;
    i2s_chan_handle_t rx_handle;
    
    // 初始化I2S通道
    i2s_new_channel(&i2s_config, &tx_handle, &rx_handle);
    i2s_channel_enable(rx_handle);
}

// 设置保存路径示例
inmp411_set_save_path("/sdcard/recording.wav");  // 或Windows路径如"C:/Users/name/Desktop/recording.wav"

// 开始录音并保存到文件
inmp411_start_record(true);  // 参数true表示启用文件保存
#ifndef _OV7670_DRIVER_H_
#define _OV7670_DRIVER_H_

#include "esp_camera.h"

// OV7670引脚配置
#define OV7670_PIN_PWDN    -1  // 不使用
#define OV7670_PIN_RESET   -1  // 不使用
#define OV7670_PIN_XCLK    GPIO_NUM_18
#define OV7670_PIN_SIOD    GPIO_NUM_5  // I2C SDA
#define OV7670_PIN_SIOC    GPIO_NUM_4  // I2C SCL
#define OV7670_PIN_D7      GPIO_NUM_13
#define OV7670_PIN_D6      GPIO_NUM_12
#define OV7670_PIN_D5      GPIO_NUM_14
#define OV7670_PIN_D4      GPIO_NUM_27
#define OV7670_PIN_D3      GPIO_NUM_26
#define OV7670_PIN_D2      GPIO_NUM_28
#define OV7670_PIN_D1      GPIO_NUM_33
#define OV7670_PIN_D0      GPIO_NUM_32
#define OV7670_PIN_VSYNC   GPIO_NUM_35
#define OV7670_PIN_HREF    GPIO_NUM_19
#define OV7670_PIN_PCLK    GPIO_NUM_34

/**
 * @brief 初始化OV7670摄像头
 * @return esp_err_t 返回初始化状态
 */
esp_err_t my_ov7670_init();

/**
 * @brief 获取摄像头帧缓冲区
 * @return camera_fb_t* 返回帧缓冲区指针
 */
camera_fb_t* my_ov7670_capture();

/**
 * @brief 释放帧缓冲区
 * @param fb 帧缓冲区指针
 */
void my_ov7670_return_fb(camera_fb_t* fb);

#endif
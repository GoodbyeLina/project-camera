#ifndef INMP411_DRIVER_H
#define INMP411_DRIVER_H

#include "driver/i2s_std.h"
#include "driver/i2s_common.h"

/**
 * @brief 初始化INMP411麦克风I2S接口
 */
void inmp411_init();

/**
 * @brief 开始录音任务
 * @param save_to_pc 是否保存到电脑
 */
void inmp411_start_record(bool save_to_pc);

/**
 * @brief 设置保存路径
 * @param path 电脑上的保存路径
 */
void inmp411_set_save_path(const char* path);

#endif // INMP411_DRIVER_H
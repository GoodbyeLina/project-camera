#include <stdio.h>
#include "inmp411_driver.h"

void app_main(void)
{
    inmp411_init();
    inmp411_set_save_path("C:/Users/97978/Desktop/recording.wav"); // 修改为实际用户名
    inmp411_start_record(true); // 启用保存到电脑功能
}
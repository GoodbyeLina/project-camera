#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "inmp411_driver.h"
#include "ov7670_driver.h"
#include "serial_comm.h"

#define TEST_MODE 0  // 1=测试模式 0=正常摄像头模式


static esp_err_t init_camera(void)
{
    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}

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
    // inmp411_set_save_path("C:/Users/97978/Desktop/recording.wav");
    inmp411_start_record(true);

    if(ESP_OK != init_camera()) {
        return;
    }

    while (1)
    {
        ESP_LOGI(TAG, "Taking picture...");
        camera_fb_t *pic = esp_camera_fb_get();

        // use pic->buf to access the image
        ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
        esp_camera_fb_return(pic);

        vTaskDelay(5000 / portTICK_RATE_MS);
    }

#endif
}
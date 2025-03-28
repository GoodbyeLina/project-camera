#include "ov7670_driver.h"
#include "esp_log.h"

static const char* TAG = "OV7670";

// 默认摄像头配置
static camera_config_t camera_config = {
    .pin_pwdn = OV7670_PIN_PWDN,
    .pin_reset = OV7670_PIN_RESET,
    .pin_xclk = OV7670_PIN_XCLK,
    .pin_sccb_sda = OV7670_PIN_SIOD,
    .pin_sccb_scl = OV7670_PIN_SIOC,
    .pin_d7 = OV7670_PIN_D7,
    .pin_d6 = OV7670_PIN_D6,
    .pin_d5 = OV7670_PIN_D5,
    .pin_d4 = OV7670_PIN_D4,
    .pin_d3 = OV7670_PIN_D3,
    .pin_d2 = OV7670_PIN_D2,
    .pin_d1 = OV7670_PIN_D1,
    .pin_d0 = OV7670_PIN_D0,
    .pin_vsync = OV7670_PIN_VSYNC,
    .pin_href = OV7670_PIN_HREF,
    .pin_pclk = OV7670_PIN_PCLK,

    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_RGB565,
    .frame_size = FRAMESIZE_QVGA,
    .jpeg_quality = 12,
    .fb_count = 1,
    .grab_mode = CAMERA_GRAB_LATEST
};

esp_err_t my_ov7670_init() {
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return err;
    }

    // 配置OV7670传感器参数
    sensor_t *s = esp_camera_sensor_get();
    if (s->id.PID == OV7670_PID) {
        s->set_vflip(s, 1);  // 垂直翻转
        s->set_hmirror(s, 1); // 水平镜像
    }

    return ESP_OK;
}

camera_fb_t* my_ov7670_capture() {
    return esp_camera_fb_get();
}

void my_ov7670_return_fb(camera_fb_t* fb) {
    esp_camera_fb_return(fb);
}
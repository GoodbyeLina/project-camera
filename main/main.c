#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "inmp411_driver.h"
#include "ov7670_driver.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define TEST_MODE 0  // 1=测试模式 0=正常摄像头模式
#define STREAM_FPS 15


// 视频流处理函数
static esp_err_t stream_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");
    
    // 视频分段保存相关变量
    static int sock = -1;
    static uint32_t segment_count = 0;
    static time_t segment_start = 0;
    time_t now = time(NULL);
    
    // 每10秒创建新连接
    if (sock == -1 || now - segment_start >= 10) {
        if (sock != -1) {
            close(sock);
        }
        
        // 连接到电脑的接收服务
        struct sockaddr_in server_addr;
        sock = socket(AF_INET, SOCK_STREAM, 0);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(8080);
        server_addr.sin_addr.s_addr = inet_addr("192.168.1.199"); // 使用用户提供的电脑IP
        
        if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            ESP_LOGE("STREAM", "Failed to connect to %s:%d",
                   inet_ntoa(server_addr.sin_addr),
                   ntohs(server_addr.sin_port));
            sock = -1;
        } else {
            segment_start = now;
            segment_count++;
        }
    }

    while (1) {
        camera_fb_t *pic = esp_camera_fb_get();
        if (!pic) {
            continue;
        }

        size_t jpg_buf_len = 0;
        uint8_t *jpg_buf = NULL;
        if(frame2jpg(pic, 80, &jpg_buf, &jpg_buf_len)) {
            char part_buf[64];
            sprintf(part_buf, "\r\n--frame\r\nContent-Type: image/jpeg\r\n\r\n");
            httpd_resp_send_chunk(req, part_buf, strlen(part_buf));
            httpd_resp_send_chunk(req, (const char *)jpg_buf, jpg_buf_len);
            
            // 发送到电脑
            if (sock != -1) {
                send(sock, part_buf, strlen(part_buf), 0);
                send(sock, jpg_buf, jpg_buf_len, 0);
            }
            
            free(jpg_buf);
        }
        esp_camera_fb_return(pic);
        vTaskDelay(1000 / STREAM_FPS / portTICK_PERIOD_MS);
    }
    
    if (sock != -1) {
        close(sock);
    }
    return ESP_OK;
}

// JPEG图像请求处理
static esp_err_t jpeg_handler(httpd_req_t *req) {
    ESP_LOGI("HTTP", "Received image request");
    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) {
        ESP_LOGE("HTTP", "Failed to capture image");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    ESP_LOGI("HTTP", "Captured image size: %d bytes", pic->len);

    // 转换YUV为JPEG
    size_t jpg_buf_len = 0;
    uint8_t *jpg_buf = NULL;
    bool jpeg_converted = frame2jpg(pic, 80, &jpg_buf, &jpg_buf_len);
    if(!jpeg_converted) {
        ESP_LOGE("HTTP", "JPEG compression failed");
        esp_camera_fb_return(pic);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_send(req, (const char *)jpg_buf, jpg_buf_len);
    
    free(jpg_buf);

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_send(req, (const char *)pic->buf, pic->len);
    
    esp_camera_fb_return(pic);
    return ESP_OK;
}

// WiFi事件处理
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
       esp_wifi_connect();
       ESP_LOGI("WIFI", "Retry to connect to the AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("WIFI", "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        
        // 启动Web服务器
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        httpd_handle_t server = NULL;
        
        httpd_uri_t jpeg_uri = {
            .uri = "/camera.jpg",
            .method = HTTP_GET,
            .handler = jpeg_handler,
            .user_ctx = NULL
        };

        httpd_uri_t stream_uri = {
            .uri = "/video",
            .method = HTTP_GET,
            .handler = stream_handler,
            .user_ctx = NULL
        };

        if (httpd_start(&server, &config) == ESP_OK) {
            httpd_register_uri_handler(server, &jpeg_uri);
            httpd_register_uri_handler(server, &stream_uri);
            ESP_LOGI("WIFI", "HTTP server started");
        } else {
            ESP_LOGE("WIFI", "Failed to start HTTP server");
        }
    }
}

// 初始化WiFi
void init_wifi() {
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 初始化TCP/IP栈
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // WiFi配置
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件处理
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    // 设置WiFi配置
    wifi_config_t wifi_config = {
        .sta = {
        .ssid = "MiFi-AC0D",
        .password = "1234567890",
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

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



void app_main(void) {

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

    // 初始化WiFi
    init_wifi();

    ESP_LOGI("MAIN", "Entering main loop");

    if(ESP_OK != init_camera()) {
        return;
    }

    while (1) {
        ESP_LOGI(TAG, "Taking picture...");
        camera_fb_t *pic = esp_camera_fb_get();

        // 发送图像数据
        if(pic) {
            ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
            esp_camera_fb_return(pic);
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

#endif
}
idf_component_register(
    SRCS "main.c" "inmp411_driver.c" "ov7670_driver.c" "serial_comm.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp32-camera esp_http_server esp_wifi nvs_flash lwip
)
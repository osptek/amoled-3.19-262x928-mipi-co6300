/*
 * SPDX-FileCopyrightText: Copyright 2026 OSPTEK
 * SPDX-License-Identifier: CC-BY-4.0
 *
 * https://github.com/osptek
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_cst3530.h"

static const char *TAG = "TOUCH_TEST";

/* ===================== 根据你的硬件修改这里 ===================== */
#define I2C_PORT            I2C_NUM_0
#define I2C_SCL_GPIO        GPIO_NUM_8      // 修改为实际 SCL
#define I2C_SDA_GPIO        GPIO_NUM_7      // 修改为实际 SDA
#define TOUCH_RST_GPIO      GPIO_NUM_NC     // 有复位脚就填，没有就 GPIO_NUM_NC
#define TOUCH_INT_GPIO      GPIO_NUM_NC     // 有中断脚就填，没有就 GPIO_NUM_NC（轮询模式）

#define TOUCH_X_MAX         262             // 屏幕宽度
#define TOUCH_Y_MAX         928             // 屏幕高度
#define I2C_FREQ_HZ         400000          // 推荐 400kHz
/* ============================================================== */

static i2c_master_bus_handle_t i2c_bus = NULL;
static i2c_master_dev_handle_t i2c_dev = NULL;
static esp_lcd_touch_handle_t  tp     = NULL;

static esp_err_t touch_init(void)
{
    /* 1. 创建 I2C 总线 */
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,   // 内部上拉，外部有上拉可关掉
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &i2c_bus));
    ESP_LOGI(TAG, "I2C bus created");

    /* 2. 添加 CST3530 设备 */
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ESP_LCD_TOUCH_IO_I2C_CST3530_ADDRESS,  // 0x58
        .scl_speed_hz = I2C_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_cfg, &i2c_dev));
    ESP_LOGI(TAG, "CST3530 device added (addr=0x%02X)", ESP_LCD_TOUCH_IO_I2C_CST3530_ADDRESS);

    /* 3. 触摸配置 */
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = TOUCH_X_MAX,
        .y_max = TOUCH_Y_MAX,
        .rst_gpio_num = TOUCH_RST_GPIO,
        .int_gpio_num = TOUCH_INT_GPIO,
        .levels = {
            .reset = 0,         // 复位有效电平（低有效常见）
            .interrupt = 0,     // 中断有效电平
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    /* 4. 创建触摸句柄（使用你提供的直接 i2c_master 接口） */
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst3530(i2c_dev, &tp_cfg, &tp));
    ESP_LOGI(TAG, "CST3530 touch initialized successfully");

    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== CST3530 Touch Test Start ===");

    ESP_ERROR_CHECK(touch_init());

    esp_lcd_touch_point_data_t points[5];   // 最多支持 5 点
    uint8_t point_num = 0;

    while (1) {
        /* 必须定期调用 read_data */
        esp_lcd_touch_read_data(tp);

        /* 使用新 API 获取触摸数据 */
        esp_err_t ret = esp_lcd_touch_get_data(tp, points, &point_num, 5);
        if (ret == ESP_OK && point_num > 0) {
            for (int i = 0; i < point_num; i++) {
                ESP_LOGI(TAG, "Point[%d]: X=%4d  Y=%4d  Strength=%3d  TrackID=%d",
                         i,
                         points[i].x,
                         points[i].y,
                         points[i].strength,
                         points[i].track_id);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));   // 50Hz 轮询
    }
}

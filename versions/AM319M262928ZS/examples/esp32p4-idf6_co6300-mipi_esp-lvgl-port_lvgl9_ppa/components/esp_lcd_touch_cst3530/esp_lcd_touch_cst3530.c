/*
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * CST3530 触摸驱动 - 直接使用新版 i2c_master（绕过 panel_io）
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_lcd_touch.h"
#include "esp_rom_sys.h"

#include "esp_lcd_touch_cst3530.h"

#ifdef CONFIG_ESP_LCD_TOUCH_MAX_POINTS
#define POINT_NUM_MAX CONFIG_ESP_LCD_TOUCH_MAX_POINTS
#else
#define POINT_NUM_MAX (5)
#endif

#define DATA_START_REG   (0xD0070000)
#define DATA_CLEAR_REG   (0xD00002AB)

#define I2C_TIMEOUT_MS   (80)

static const char *TAG = "CST3530";

/* 私有结构体 */
typedef struct {
    esp_lcd_touch_t base;                   /* 必须放在第一位 */
    i2c_master_dev_handle_t i2c_dev;
} cst3530_touch_t;

static esp_err_t read_data(esp_lcd_touch_handle_t tp);
static bool get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength,
                   uint8_t *point_num, uint8_t max_point_num);
static esp_err_t del(esp_lcd_touch_handle_t tp);

static esp_err_t touch_cst3530_read_reg(cst3530_touch_t *touch, uint32_t reg, uint8_t *data, size_t len);
static esp_err_t touch_cst3530_write_reg(cst3530_touch_t *touch, uint32_t reg);
static esp_err_t reset(esp_lcd_touch_handle_t tp);

esp_err_t esp_lcd_touch_new_i2c_cst3530(i2c_master_dev_handle_t i2c_dev,
                                        const esp_lcd_touch_config_t *config,
                                        esp_lcd_touch_handle_t *tp)
{
    ESP_RETURN_ON_FALSE(i2c_dev, ESP_ERR_INVALID_ARG, TAG, "Invalid i2c_dev");
    ESP_RETURN_ON_FALSE(config, ESP_ERR_INVALID_ARG, TAG, "Invalid config");
    ESP_RETURN_ON_FALSE(tp, ESP_ERR_INVALID_ARG, TAG, "Invalid touch handle");

    esp_err_t ret = ESP_OK;

    cst3530_touch_t *touch = calloc(1, sizeof(cst3530_touch_t));
    ESP_RETURN_ON_FALSE(touch, ESP_ERR_NO_MEM, TAG, "Touch handle malloc failed");

    touch->i2c_dev = i2c_dev;

    /* 填充标准接口 */
    touch->base.read_data = read_data;
    touch->base.get_xy = get_xy;
    touch->base.del = del;
    touch->base.data.lock.owner = portMUX_FREE_VAL;
    memcpy(&touch->base.config, config, sizeof(esp_lcd_touch_config_t));

    /* 中断 GPIO */
    if (touch->base.config.int_gpio_num != GPIO_NUM_NC) {
        const gpio_config_t int_gpio_config = {
            .mode = GPIO_MODE_INPUT,
            .intr_type = (touch->base.config.levels.interrupt ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE),
            .pin_bit_mask = BIT64(touch->base.config.int_gpio_num)
        };
        ESP_GOTO_ON_ERROR(gpio_config(&int_gpio_config), err, TAG, "GPIO intr config failed");

        if (touch->base.config.interrupt_callback) {
            esp_lcd_touch_register_interrupt_callback(&touch->base, touch->base.config.interrupt_callback);
        }
    }

    /* 复位 GPIO */
    if (touch->base.config.rst_gpio_num != GPIO_NUM_NC) {
        const gpio_config_t rst_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = BIT64(touch->base.config.rst_gpio_num)
        };
        ESP_GOTO_ON_ERROR(gpio_config(&rst_gpio_config), err, TAG, "GPIO reset config failed");
    }

    /* 硬件复位 */
    ESP_GOTO_ON_ERROR(reset(&touch->base), err, TAG, "Reset failed");

    *tp = &touch->base;
    ESP_LOGI(TAG, "CST3530 initialized (direct i2c_master, interrupt/polling supported)");
    return ESP_OK;

err:
    if (touch) {
        del(&touch->base);
    }
    ESP_LOGE(TAG, "Initialization failed!");
    return ret;
}

static esp_err_t read_data(esp_lcd_touch_handle_t tp)
{
    cst3530_touch_t *touch = (cst3530_touch_t *)tp;
    uint8_t buf[64] = {0};

    /* 读取芯片坐标寄存器 */
    esp_err_t ret = touch_cst3530_read_reg(touch, DATA_START_REG, buf, 64);
    if (ret != ESP_OK) {
        /* I2C 偶然超时时，不直接清零，保持上一次数据 1 帧，避免 LVGL 判定为 Press Lost */
        return ESP_OK;
    }

    uint8_t report_typ = buf[2];
    uint8_t finger_num = buf[3] & 0x0F;
    uint8_t key_num    = (buf[3] & 0xF0) >> 4;

    /* 清除报告（忽略写失败） */
    (void)touch_cst3530_write_reg(touch, DATA_CLEAR_REG);

    portENTER_CRITICAL(&tp->data.lock);

    if (report_typ == 0xff && finger_num > 0) {
        uint8_t valid_points = 0;
        for (int i = 0; i < finger_num && valid_points < POINT_NUM_MAX; i++) {
            int index = (key_num + i) * 5;
            uint16_t x = buf[index + 4] + ((uint16_t)(buf[index + 7] & 0x0F) << 8);
            uint16_t y = buf[index + 5] + ((uint16_t)(buf[index + 7] & 0xF0) << 4);
            uint16_t strength = buf[index + 6];

            if (x < tp->config.x_max && y < tp->config.y_max && strength > 0) {
                tp->data.coords[valid_points].x = x;
                tp->data.coords[valid_points].y = y;
                tp->data.coords[valid_points].strength = strength;
                valid_points++;
            }
        }
        tp->data.points = valid_points;
    } else {
        /* 手指正常抬起：清零点数 */
        tp->data.points = 0;
    }

    portEXIT_CRITICAL(&tp->data.lock);
    return ESP_OK;
}

static bool get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength,
                   uint8_t *point_num, uint8_t max_point_num)
{
    portENTER_CRITICAL(&tp->data.lock);

    *point_num = (tp->data.points > max_point_num ? max_point_num : tp->data.points);

    for (size_t i = 0; i < *point_num; i++) {
        x[i] = tp->data.coords[i].x;
        y[i] = tp->data.coords[i].y;
        if (strength) {
            strength[i] = tp->data.coords[i].strength;
        }
    }

    /* 实时清空已获取的点数，确保无触控时正确释放 */
    tp->data.points = 0;

    portEXIT_CRITICAL(&tp->data.lock);

    return (*point_num > 0);
}

static esp_err_t del(esp_lcd_touch_handle_t tp)
{
    cst3530_touch_t *touch = (cst3530_touch_t *)tp;

    if (tp->config.int_gpio_num != GPIO_NUM_NC) {
        gpio_reset_pin(tp->config.int_gpio_num);
        if (tp->config.interrupt_callback) {
            gpio_isr_handler_remove(tp->config.int_gpio_num);
        }
    }
    if (tp->config.rst_gpio_num != GPIO_NUM_NC) {
        gpio_reset_pin(tp->config.rst_gpio_num);
    }

    free(touch);
    return ESP_OK;
}

static esp_err_t reset(esp_lcd_touch_handle_t tp)
{
    if (tp->config.rst_gpio_num != GPIO_NUM_NC) {
        ESP_RETURN_ON_ERROR(gpio_set_level(tp->config.rst_gpio_num, tp->config.levels.reset), TAG, "GPIO set level failed");
        vTaskDelay(pdMS_TO_TICKS(200));
        ESP_RETURN_ON_ERROR(gpio_set_level(tp->config.rst_gpio_num, !tp->config.levels.reset), TAG, "GPIO set level failed");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    return ESP_OK;
}

/* ==================== 底层 I2C 读写 ==================== */

static esp_err_t touch_cst3530_read_reg(cst3530_touch_t *touch, uint32_t reg, uint8_t *data, size_t len)
{
    uint8_t cmd[4] = {
        (reg >> 24) & 0xFF,
        (reg >> 16) & 0xFF,
        (reg >> 8)  & 0xFF,
        (reg)       & 0xFF
    };

    return i2c_master_transmit_receive(touch->i2c_dev, cmd, 4, data, len, I2C_TIMEOUT_MS);
}

static esp_err_t touch_cst3530_write_reg(cst3530_touch_t *touch, uint32_t reg)
{
    uint8_t cmd[4] = {
        (reg >> 24) & 0xFF,
        (reg >> 16) & 0xFF,
        (reg >> 8)  & 0xFF,
        (reg)       & 0xFF
    };

    return i2c_master_transmit(touch->i2c_dev, cmd, 4, I2C_TIMEOUT_MS);
}
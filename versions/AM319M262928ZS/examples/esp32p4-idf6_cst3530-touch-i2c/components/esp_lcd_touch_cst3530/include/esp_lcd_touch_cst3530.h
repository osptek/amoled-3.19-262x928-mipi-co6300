/*
 * SPDX-FileCopyrightText: 2015-2024 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileCopyrightText: 2025 Waveshare
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ESP LCD touch: CST3530 (直接使用 i2c_master，绕过 panel_io)
 */

#pragma once

#include "esp_lcd_touch.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 创建 CST3530 触摸驱动（直接使用新版 i2c_master）
 *
 * @param i2c_dev   已经通过 i2c_master_bus_add_device() 添加的设备句柄
 * @param config    触摸配置
 * @param out_touch 返回的触摸句柄
 */
esp_err_t esp_lcd_touch_new_i2c_cst3530(i2c_master_dev_handle_t i2c_dev,
                                        const esp_lcd_touch_config_t *config,
                                        esp_lcd_touch_handle_t *out_touch);

#define ESP_LCD_TOUCH_IO_I2C_CST3530_ADDRESS (0x58)

#ifdef __cplusplus
}
#endif
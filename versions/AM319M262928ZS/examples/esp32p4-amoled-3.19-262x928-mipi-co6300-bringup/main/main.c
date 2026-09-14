/*
 * SPDX-FileCopyrightText: Copyright 2026 OSPTEK
 * SPDX-License-Identifier: CC-BY-4.0
 *
 * https://github.com/osptek
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "driver/i2c.h"

#include "esp_attr.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_co6300.h"
#include "esp_lcd_touch_cst3530.h"
#include "esp_ldo_regulator.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_demos.h"

static const char *TAG = "Main";

#define MIPI_DSI_DPI_CLK_MHZ 16

#define MIPI_DSI_LCD_H_RES 262
#define MIPI_DSI_LCD_V_RES 928

#define MIPI_DSI_LCD_HSYNC 4
#define MIPI_DSI_LCD_HBP 32
#define MIPI_DSI_LCD_HFP 32
#define MIPI_DSI_LCD_VSYNC 4
#define MIPI_DSI_LCD_VBP 8
#define MIPI_DSI_LCD_VFP 8

#define TEST_MIPI_DSI_PHY_PWR_LDO_CHAN 3
#define TEST_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500

#define EXAMPLE_PIN_NUM_TOUCH_SCL (GPIO_NUM_8)
#define EXAMPLE_PIN_NUM_TOUCH_SDA (GPIO_NUM_7)
#define EXAMPLE_PIN_NUM_TOUCH_RST (GPIO_NUM_16)
#define EXAMPLE_PIN_NUM_TOUCH_INT (GPIO_NUM_17)

#define LCD_VCI_EN_GPIO GPIO_NUM_22

#define TOUCH_HOST I2C_NUM_0

#define LCD_DRAW_BUFF_HEIGHT (120)  // 降低缓冲区高度，节省内存

static esp_lcd_panel_io_handle_t mipi_dbi_io = NULL;
static esp_lcd_panel_handle_t mipi_dpi_panel = NULL;
static esp_lcd_touch_handle_t tp = NULL;

static lv_display_t *lvgl_disp = NULL;

static const co6300_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xFE, (uint8_t[]){0x20}, 1, 0},
    {0xF4, (uint8_t[]){0x5A}, 1, 0},
    {0xF5, (uint8_t[]){0x59}, 1, 0},
    {0xFE, (uint8_t[]){0x80}, 1, 0},
    {0x03, (uint8_t[]){0x00}, 1, 0},
    {0xFE, (uint8_t[]){0x00}, 1, 0},
    {0x35, (uint8_t[]){0x00}, 1, 0},
    {0x3A, (uint8_t[]){0x77}, 1, 0},
    {0x2A, (uint8_t[]){0x00, 0x00, 0x01, 0x05}, 4, 0},
    {0x2B, (uint8_t[]){0x00, 0x00, 0x03, 0x9F}, 4, 0},
    {0x53, (uint8_t[]){0x20}, 1, 0},
    {0x51, (uint8_t[]){0xFF}, 1, 0},
    {0x63, (uint8_t[]){0xFF}, 1, 0},
    {0x11, NULL, 0, 60},
    {0x29, NULL, 0, 0},
};

static void lcd_vci_en_init(void) {
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << LCD_VCI_EN_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);

    gpio_set_level(LCD_VCI_EN_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(60));

    gpio_set_level(LCD_VCI_EN_GPIO, 1);

    vTaskDelay(pdMS_TO_TICKS(50));
}

static void i2c_scan(void) {
    ESP_LOGI(TAG, "Scanning I2C bus...");
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at address 0x%02x", addr);
        }
    }
    ESP_LOGI(TAG, "I2C scan completed.");
}

esp_err_t app_lcd_init() {
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus;
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;

    ESP_LOGI(TAG, "MIPI DSI PHY Powered on");
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = TEST_MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = TEST_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));

    esp_lcd_dsi_bus_config_t bus_config = {
        .bus_id = 0,
        .num_data_lanes = 1,
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
        .lane_bit_rate_mbps = 360,
    };
    ESP_LOGI(TAG, "esp_lcd_new_dsi_bus!");
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus));

    esp_lcd_dbi_io_config_t dbi_config = {
        .virtual_channel = 0,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_LOGI(TAG, "esp_lcd_new_panel_io_dbi!");
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io));

    esp_lcd_dpi_panel_config_t dpi_config = {
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = 16,
        .virtual_channel = 0,
        .pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB565,
        .num_fbs = 1,
        .video_timing =
            {
                .h_size = MIPI_DSI_LCD_H_RES,
                .v_size = MIPI_DSI_LCD_V_RES,
                .hsync_back_porch = MIPI_DSI_LCD_HBP,
                .hsync_pulse_width = MIPI_DSI_LCD_HSYNC,
                .hsync_front_porch = MIPI_DSI_LCD_HFP,
                .vsync_back_porch = MIPI_DSI_LCD_VBP,
                .vsync_pulse_width = MIPI_DSI_LCD_VSYNC,
                .vsync_front_porch = MIPI_DSI_LCD_VFP,
            },
        .flags =
            {
                .use_dma2d = true,
            }
    };

    co6300_vendor_config_t vendor_config = {0};
    vendor_config.init_cmds = lcd_init_cmds;
    vendor_config.init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]);
    vendor_config.flags.use_mipi_interface = 1;
    vendor_config.mipi_config.dsi_bus = mipi_dsi_bus;
    vendor_config.mipi_config.dpi_config = &dpi_config;

    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = 6,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .vendor_config = &vendor_config,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_co6300(mipi_dbi_io, &lcd_dev_config, &mipi_dpi_panel));
    esp_lcd_panel_reset(mipi_dpi_panel);
    esp_lcd_panel_init(mipi_dpi_panel);

    assert(mipi_dbi_io);
    assert(mipi_dpi_panel);

    return ESP_OK;
}

esp_err_t app_touch_init() {
    ESP_LOGI(TAG, "Initialize I2C bus");

    // 确保触摸 RST 释放，否则 I2C 无应答
    const gpio_config_t rst_cfg = {
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_TOUCH_RST,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&rst_cfg);
    gpio_set_level(EXAMPLE_PIN_NUM_TOUCH_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(20));

    const i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = EXAMPLE_PIN_NUM_TOUCH_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = EXAMPLE_PIN_NUM_TOUCH_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400 * 1000,
    };
    ESP_ERROR_CHECK(i2c_param_config(TOUCH_HOST, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(TOUCH_HOST, i2c_conf.mode, 0, 0, 0));
    i2c_scan();

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST3530_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)TOUCH_HOST, &tp_io_config, &tp_io_handle));
    // 给 INT 脚上拉，避免悬空噪声误报
    gpio_set_pull_mode(EXAMPLE_PIN_NUM_TOUCH_INT, GPIO_PULLUP_ONLY);

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = MIPI_DSI_LCD_H_RES,
        .y_max = MIPI_DSI_LCD_V_RES,
        .rst_gpio_num = EXAMPLE_PIN_NUM_TOUCH_RST,
        .int_gpio_num = EXAMPLE_PIN_NUM_TOUCH_INT,
        .levels =
            {
                .reset = 0,
                .interrupt = 0,
            },
        .flags =
            {
                .swap_xy = 0,
                .mirror_x = 0,
                .mirror_y = 0,
            },
    };
    ESP_LOGI(TAG, "Initialize touch controller");
    return esp_lcd_touch_new_i2c_cst3530(tp_io_handle, &tp_cfg, &tp);
}

esp_err_t app_lvgl_init() {
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,      /* LVGL task priority */
        .task_stack = 4096 * 2,  /* LVGL task stack size */
        .task_affinity = -1,     /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500,
        .timer_period_ms = 5
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = mipi_dbi_io,
        .panel_handle = mipi_dpi_panel,
        .buffer_size = MIPI_DSI_LCD_H_RES * LCD_DRAW_BUFF_HEIGHT,
        .double_buffer = true,
        .hres = MIPI_DSI_LCD_H_RES,
        .vres = MIPI_DSI_LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation =
            {
                .swap_xy = false,
                .mirror_x = false,
                .mirror_y = false,
            },
        .flags =
            {
                .buff_dma = true,
                .buff_spiram = false,
                .swap_bytes = false,
            }
    };
    const lvgl_port_display_dsi_cfg_t dsi_cfg = {
        .flags =
            {
                .avoid_tearing = false,
            },
    };
    lvgl_disp = lvgl_port_add_disp_dsi(&disp_cfg, &dsi_cfg);

    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_disp,
        .handle = tp,
    };
    lvgl_port_add_touch(&touch_cfg);

    return ESP_OK;
}

void app_main(void) {
    lcd_vci_en_init();

    ESP_ERROR_CHECK(app_lcd_init());
    ESP_ERROR_CHECK(app_touch_init());
    ESP_ERROR_CHECK(app_lvgl_init());

    lvgl_port_lock(-1);
    lv_demo_widgets();
    // lv_demo_music();
    lvgl_port_unlock();
}

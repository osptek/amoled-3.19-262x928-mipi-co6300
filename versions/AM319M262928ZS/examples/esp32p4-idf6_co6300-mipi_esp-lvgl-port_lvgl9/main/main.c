/*
 * SPDX-FileCopyrightText: Copyright 2026 OSPTEK
 * SPDX-License-Identifier: CC-BY-4.0
 *
 * https://github.com/osptek
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "driver/i2c_master.h"

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

#if LV_COLOR_DEPTH == 16
#define MIPI_DSI_LANE_BITRATE_MBPS  360
#define MIPI_DPI_PX_FORMAT (LCD_COLOR_FMT_RGB565)
#define BSP_LCD_COLOR_DEPTH (16)
#define LCD_RGB_ELEMENT_ORDER LCD_RGB_ELEMENT_ORDER_BGR
#define LV_COLOR_FORMAT LV_COLOR_FORMAT_RGB565
#elif LV_COLOR_DEPTH == 24
#define MIPI_DSI_LANE_BITRATE_MBPS  480
#define MIPI_DPI_PX_FORMAT (LCD_COLOR_FMT_RGB888)
#define BSP_LCD_COLOR_DEPTH (24)
#define LCD_RGB_ELEMENT_ORDER LCD_RGB_ELEMENT_ORDER_RGB
#define LV_COLOR_FORMAT LV_COLOR_FORMAT_RGB888
#endif

#define MIPI_DSI_DPI_CLK_MHZ 16

#define EXAMPLE_LCD_H_RES 262
#define EXAMPLE_LCD_V_RES 928

#define MIPI_DSI_LCD_HSYNC 4
#define MIPI_DSI_LCD_HBP 32
#define MIPI_DSI_LCD_HFP 32
#define MIPI_DSI_LCD_VSYNC 4
#define MIPI_DSI_LCD_VBP 8
#define MIPI_DSI_LCD_VFP 8

#define TEST_MIPI_DSI_PHY_PWR_LDO_CHAN 3
#define TEST_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500
#define EXAMPLE_PIN_NUM_LCD_RST   (GPIO_NUM_NC)

/* Touch settings */
#define EXAMPLE_TOUCH_I2C_NUM       (I2C_NUM_0)
#define EXAMPLE_TOUCH_I2C_CLK_HZ    (400 * 1000)

/* LCD touch pins */
#define EXAMPLE_TOUCH_I2C_SCL       (GPIO_NUM_8)
#define EXAMPLE_TOUCH_I2C_SDA       (GPIO_NUM_7)
#define EXAMPLE_TOUCH_RST           (GPIO_NUM_NC)
#define EXAMPLE_TOUCH_INT           (GPIO_NUM_23)

#define LCD_VCI_EN_GPIO GPIO_NUM_22

static esp_lcd_panel_io_handle_t mipi_dbi_io = NULL;
static esp_lcd_panel_handle_t mipi_dpi_panel = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

static lv_display_t *lvgl_disp = NULL;

static const co6300_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xFE, (uint8_t[]){0x20}, 1, 0},
    {0xF4, (uint8_t[]){0x5A}, 1, 0},
    {0xF5, (uint8_t[]){0x59}, 1, 0},
    {0xFE, (uint8_t[]){0x80}, 1, 0},
    {0x03, (uint8_t[]){0x00}, 1, 0},
    {0xFE, (uint8_t[]){0x00}, 1, 0},
    {0x35, (uint8_t[]){0x00}, 1, 0},

#if LV_COLOR_DEPTH == 16
    {0x3A, (uint8_t[]){0x55}, 1, 0},
#elif LV_COLOR_DEPTH == 24
    {0x3A, (uint8_t[]){0x77}, 1, 0},
#endif

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
        .phy_clk_src = 0,
        .lane_bit_rate_mbps = MIPI_DSI_LANE_BITRATE_MBPS,
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
        .dpi_clock_freq_mhz = MIPI_DSI_DPI_CLK_MHZ,
        .virtual_channel = 0,
        .in_color_format = MIPI_DPI_PX_FORMAT,
        .num_fbs = 1,
        .video_timing =
            {
                .h_size = EXAMPLE_LCD_H_RES,
                .v_size = EXAMPLE_LCD_V_RES,
                .hsync_back_porch = MIPI_DSI_LCD_HBP,
                .hsync_pulse_width = MIPI_DSI_LCD_HSYNC,
                .hsync_front_porch = MIPI_DSI_LCD_HFP,
                .vsync_back_porch = MIPI_DSI_LCD_VBP,
                .vsync_pulse_width = MIPI_DSI_LCD_VSYNC,
                .vsync_front_porch = MIPI_DSI_LCD_VFP,
            },
    };

    co6300_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
        .flags.use_mipi_interface = 1,
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };

    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER,
        .bits_per_pixel = BSP_LCD_COLOR_DEPTH,
        .vendor_config = &vendor_config,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_co6300(mipi_dbi_io, &lcd_dev_config, &mipi_dpi_panel));
    esp_lcd_panel_reset(mipi_dpi_panel);
    esp_lcd_panel_init(mipi_dpi_panel);

    assert(mipi_dbi_io);
    assert(mipi_dpi_panel);

    return ESP_OK;
}

static esp_err_t app_touch_init(void)
{
    /* 1. 初始化 I2C 总线（新版 i2c_master） */
    const i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = EXAMPLE_TOUCH_I2C_NUM,
        .scl_io_num = EXAMPLE_TOUCH_I2C_SCL,
        .sda_io_num = EXAMPLE_TOUCH_I2C_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t i2c_bus = NULL;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus));

    /* 2. 添加 CST3530 设备到总线 */
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ESP_LCD_TOUCH_IO_I2C_CST3530_ADDRESS,  // 0x58
        .scl_speed_hz = EXAMPLE_TOUCH_I2C_CLK_HZ,               // 100kHz
    };
    i2c_master_dev_handle_t touch_dev = NULL;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_cfg, &touch_dev));

    /* 3. 初始化触摸驱动（直接传设备句柄，绕过 panel_io） */
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,
        .y_max = EXAMPLE_LCD_V_RES,
        .rst_gpio_num = EXAMPLE_TOUCH_RST,
        .int_gpio_num = EXAMPLE_TOUCH_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    return esp_lcd_touch_new_i2c_cst3530(touch_dev, &tp_cfg, &touch_handle);
}

esp_err_t app_lvgl_init() {
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,      /* LVGL task priority */
        .task_stack = 4096 * 4,  /* LVGL task stack size */
        .task_affinity = -1,     /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500,
        .timer_period_ms = 5
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = mipi_dbi_io,
        .panel_handle = mipi_dpi_panel,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES,
        .double_buffer = true,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT,
        .rotation =
            {
                .swap_xy = false,
                .mirror_x = false,
                .mirror_y = false,
            },
        .flags =
            {
            .buff_dma = false,
            .buff_spiram = true,
            .sw_rotate = true,// true: 软件; false: 硬件
            .swap_bytes = false,
            .full_refresh = false,
            .direct_mode = false,
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
        .handle = touch_handle,
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

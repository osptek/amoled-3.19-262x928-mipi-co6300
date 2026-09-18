<p align="left"><img alt="OSPTEK" src="./images/logo.png" width="200" /></p>

<h1 align="center">OSPTEK 3.19″ AMOLED 262×928（CO6300 · MIPI）</h1>

<p align="center"><b>条形 AMOLED 模组 · MIPI DSI · CO6300</b></p>

<p align="center"><a href="./README_EN.md">English</a> | 简体中文 · <a href="../../README.md">规格族索引</a></p>

<p align="center">
  <img alt="Size: 3.19 inch" src="https://img.shields.io/badge/Size-3.19%22-3498DB?style=flat-square" />
  <img alt="Resolution: 262x928" src="https://img.shields.io/badge/Resolution-262%C3%97928-8E44AD?style=flat-square" />
  <img alt="Interface: MIPI" src="https://img.shields.io/badge/Interface-MIPI-27AE60?style=flat-square" />
  <img alt="Driver: CO6300" src="https://img.shields.io/badge/Driver-CO6300-E7352C?style=flat-square" />
</p>

<p align="center"><img alt="OSPTEK 3.19 寸 262×928 AMOLED MIPI 模组（CO6300）宣传图" src="./images/product.png" width="640" /></p>

## 目录

- [产品简介](#产品简介)
- [规格参数](#规格参数)
- [示例工程](#示例工程)
- [仓库结构](#仓库结构)
- [相关资料](#相关资料)
- [购买链接](#购买链接)
- [技术支持](#技术支持)

---

## 产品简介

OSPTEK **3.19 寸 262×928 AMOLED** 是一款 **MIPI DSI** 接口彩色显示模组，显示驱动为 **CO6300**，触摸驱动为 **CST3530**。细长分辨率适合条形 HMI、侧边状态条与竖向信息面板等场景。

规格标识（仓库名）：`amoled-3.19-262x928-mipi-co6300`

当前模组版本：**AM319M262928ZS**。电气与外形细节以 [`docs/AM_319_M262928_ZS_3a5effae7d.pdf`](./docs/AM_319_M262928_ZS_3a5effae7d.pdf) 为准。

## 规格参数

| 项目 | 规格 |
| ---- | ---- |
| 尺寸 | 3.19 英寸 |
| 类型 | AMOLED（彩色） |
| 分辨率 | 262×928 |
| 接口 | MIPI DSI |
| 驱动 IC | CO6300 |
| 触摸驱动 | CST3530 |

> 完整外形尺寸、FPC 定义、供电与时序以产品规格书 / 驱动手册为准。

## 示例工程

| 说明 | 路径 |
| ---- | ---- |
| ESP32-P4 · CO6300 MIPI + CST3530 + LVGL | [`examples/esp32p4-amoled-3.19-262x928-mipi-co6300-bringup/`](./examples/esp32p4-amoled-3.19-262x928-mipi-co6300-bringup/) |
| ESP32-P4 v3.2 · ESP-IDF 6.1 · CO6300 MIPI + CST3530 + LVGL 9 | [`examples/esp32p4-idf6_co6300-mipi_esp-lvgl-port_lvgl9/`](./examples/esp32p4-idf6_co6300-mipi_esp-lvgl-port_lvgl9/) |
| ESP32-P4 v3.2 · ESP-IDF 6.1 · CO6300 MIPI + CST3530 + LVGL 9（PPA 横屏） | [`examples/esp32p4-idf6_co6300-mipi_esp-lvgl-port_lvgl9_ppa/`](./examples/esp32p4-idf6_co6300-mipi_esp-lvgl-port_lvgl9_ppa/) |
| ESP32-P4 v3.2 · ESP-IDF 6.1 · CST3530 I2C 触摸测试 | [`examples/esp32p4-idf6_cst3530-touch-i2c/`](./examples/esp32p4-idf6_cst3530-touch-i2c/) |
| ESP32-P4 v3.2 · ESP-IDF 6.1 · I2C 地址扫描 | [`examples/esp32p4-idf6_i2c-scanner/`](./examples/esp32p4-idf6_i2c-scanner/) |

## 仓库结构

```text
amoled-3.19-262x928-mipi-co6300/                                # 仓库根（导航见 ../../README.md）
└── versions/
    └── AM319M262928ZS/                                # 本料号完整资料
        ├── README.md
        ├── README_EN.md
        ├── images/
        ├── docs/
        └── examples/
```

## 相关资料

### 本产品资料

| 资料 | 链接 |
| ---- | ---- |
| 产品规格书（AM319M262928ZS） | [`docs/AM_319_M262928_ZS_3a5effae7d.pdf`](./docs/AM_319_M262928_ZS_3a5effae7d.pdf) |
| 驱动 IC 数据手册（CO6300） | [`docs/CO_6300_Datasheet_V0_00_20240827_for_customer_d6da76ca4d.pdf`](./docs/CO_6300_Datasheet_V0_00_20240827_for_customer_d6da76ca4d.pdf) |
| 触摸 IC 数据手册（CST3530） | [`docs/CST_3530_V1_0_adb72690f0.pdf`](./docs/CST_3530_V1_0_adb72690f0.pdf) |
| 初始化序列（文本） | [`docs/AM319M262928ZS_CO6300_init_BOE3.19.txt`](./docs/AM319M262928ZS_CO6300_init_BOE3.19.txt) |
| 转接板原理图 | [`docs/3.19寸262x928 AMOLED转接板.pdf`](./docs/3.19%E5%AF%B8262x928%20AMOLED%E8%BD%AC%E6%8E%A5%E6%9D%BF.pdf) |
| PCB 尺寸图（正面 · 27.00×93.00 mm） | [`images/dimensions-front.png`](./images/dimensions-front.png) |
| PCB 尺寸图（正面对角线 · 96.84 mm） | [`images/dimensions-front-diagonal.png`](./images/dimensions-front-diagonal.png) |
| PCB 尺寸图（背面 · 27.00×93.00 mm） | [`images/dimensions-back.png`](./images/dimensions-back.png) |
| PCB 尺寸图（背面连接器） | [`images/dimensions-back-connector.png`](./images/dimensions-back-connector.png) |
| 转接板联调实拍 | [`assets/adapter-board-demo.jpg`](./assets/adapter-board-demo.jpg) |

<p align="center">
  <img alt="3.19 寸 AMOLED PCB 尺寸图（正面）" src="./images/dimensions-front.png" width="220" />
  &nbsp;&nbsp;
  <img alt="3.19 寸 AMOLED PCB 尺寸图（正面对角线）" src="./images/dimensions-front-diagonal.png" width="220" />
</p>
<p align="center">
  <img alt="3.19 寸 AMOLED PCB 尺寸图（背面）" src="./images/dimensions-back.png" width="220" />
  &nbsp;&nbsp;
  <img alt="3.19 寸 AMOLED PCB 尺寸图（背面连接器）" src="./images/dimensions-back-connector.png" width="220" />
</p>

### 示例工程

- [ESP32-P4 CO6300 MIPI + CST3530 + LVGL](./examples/esp32p4-amoled-3.19-262x928-mipi-co6300-bringup/)
- [ESP32-P4 v3.2 · ESP-IDF 6.1 · CO6300 MIPI + CST3530 + LVGL 9](./examples/esp32p4-idf6_co6300-mipi_esp-lvgl-port_lvgl9/)
- [ESP32-P4 v3.2 · ESP-IDF 6.1 · CO6300 MIPI + CST3530 + LVGL 9（PPA 横屏）](./examples/esp32p4-idf6_co6300-mipi_esp-lvgl-port_lvgl9_ppa/)
- [ESP32-P4 v3.2 · ESP-IDF 6.1 · CST3530 I2C 触摸测试](./examples/esp32p4-idf6_cst3530-touch-i2c/)
- [ESP32-P4 v3.2 · ESP-IDF 6.1 · I2C 地址扫描](./examples/esp32p4-idf6_i2c-scanner/)

## 购买链接

<p align="center">
  <a href="https://shop110742373.taobao.com/"><img alt="淘宝官方店铺" src="https://img.shields.io/badge/淘宝-官方店铺-FF6A00?style=for-the-badge" /></a>
  &nbsp;&nbsp;
  <a href="https://www.aliexpress.com/store/1105701619"><img alt="速卖通官方店铺" src="https://img.shields.io/badge/速卖通-官方店铺-E62E04?style=for-the-badge&logo=aliexpress&logoColor=white" /></a>
</p>

**国内（淘宝）**

- 店铺：[鱼鹰光电工厂店](https://shop110742373.taobao.com/)

**海外（AliExpress）**

- 店铺：[OSPTEK Official Store](https://www.aliexpress.com/store/1105701619)

## 技术支持

- 技术支持 / 产品咨询：<luyu@osptek.com>
- QQ 技术交流群：**985881096**
- 公司官网：<https://osptek.com/>
- 有任何问题，都可以在本仓库 Issues 中提问

---

<p align="center"><sub>© 2026 OSPTEK 鱼鹰光电 · 本仓库资料采用 CC BY 4.0 许可</sub></p>

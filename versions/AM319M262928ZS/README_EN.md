<p align="left"><img alt="OSPTEK" src="./images/logo.png" width="200" /></p>

<h1 align="center">OSPTEK 3.19″ AMOLED 262×928 (CO6300 · MIPI)</h1>

<p align="center"><b>Bar AMOLED module · MIPI DSI · CO6300</b></p>

<p align="center">English | <a href="./README.md">简体中文</a></p>

<p align="center">
  <img alt="Size: 3.19 inch" src="https://img.shields.io/badge/Size-3.19%22-3498DB?style=flat-square" />
  <img alt="Resolution: 262x928" src="https://img.shields.io/badge/Resolution-262%C3%97928-8E44AD?style=flat-square" />
  <img alt="Interface: MIPI" src="https://img.shields.io/badge/Interface-MIPI-27AE60?style=flat-square" />
  <img alt="Driver: CO6300" src="https://img.shields.io/badge/Driver-CO6300-E7352C?style=flat-square" />
</p>

<p align="center"><img alt="OSPTEK 3.19&quot; 262×928 AMOLED MIPI module (CO6300) product image" src="./images/product.png" width="640" /></p>

## Contents

- [Overview](#overview)
- [Specifications](#specifications)
- [Sample projects](#sample-projects)
- [Repository layout](#repository-layout)
- [Resources](#resources)
- [Where to Buy](#where-to-buy)
- [Support](#support)

---

## Overview

OSPTEK **3.19″ 262×928 AMOLED** is a **MIPI DSI** color display module driven by **CO6300**, with touch IC **CST3530**. The tall aspect ratio suits bar HMIs, side status strips, and vertical info panels.

Repo id: `amoled-3.19-262x928-mipi-co6300`

Current module version: **AM319M262928ZS**. Electrical and mechanical details follow [`docs/AM_319_M262928_ZS_3a5effae7d.pdf`](./docs/AM_319_M262928_ZS_3a5effae7d.pdf).

## Specifications

| Item | Spec |
| ---- | ---- |
| Size | 3.19 inch |
| Type | AMOLED (color) |
| Resolution | 262×928 |
| Interface | MIPI DSI |
| Driver IC | CO6300 |
| Touch driver | CST3530 |

> Full outline, FPC definition, power, and timing follow the product datasheet / driver IC datasheet.

## Sample projects

| Description | Path |
| ----------- | ---- |
| ESP32-P4 · CO6300 MIPI + CST3530 + LVGL | [`examples/esp32p4-amoled-3.19-262x928-mipi-co6300-bringup/`](./examples/esp32p4-amoled-3.19-262x928-mipi-co6300-bringup/) |

## Repository layout

```text
amoled-3.19-262x928-mipi-co6300/                                # repo root (nav: ../../README_EN.md)
└── versions/
    └── AM319M262928ZS/                                # full materials for this part number
        ├── README.md
        ├── README_EN.md
        ├── images/
        ├── docs/
        └── examples/
```

## Resources

### Product files

| Resource | Link |
| -------- | ---- |
| Product datasheet (AM319M262928ZS) | [`docs/AM_319_M262928_ZS_3a5effae7d.pdf`](./docs/AM_319_M262928_ZS_3a5effae7d.pdf) |
| Driver IC datasheet (CO6300) | [`docs/CO_6300_Datasheet_V0_00_20240827_for_customer_d6da76ca4d.pdf`](./docs/CO_6300_Datasheet_V0_00_20240827_for_customer_d6da76ca4d.pdf) |
| Touch IC datasheet (CST3530) | [`docs/CST_3530_V1_0_adb72690f0.pdf`](./docs/CST_3530_V1_0_adb72690f0.pdf) |
| Init sequence (text) | [`docs/AM319M262928ZS_CO6300_init_BOE3.19.txt`](./docs/AM319M262928ZS_CO6300_init_BOE3.19.txt) |
| Adapter board schematic | [`docs/3.19寸262x928 AMOLED转接板.pdf`](./docs/3.19%E5%AF%B8262x928%20AMOLED%E8%BD%AC%E6%8E%A5%E6%9D%BF.pdf) |
| PCB outline (front · 27.00×93.00 mm) | [`images/dimensions-front.png`](./images/dimensions-front.png) |
| PCB outline (back · 27.00×93.00 mm) | [`images/dimensions-back.png`](./images/dimensions-back.png) |
| Adapter board demo photo | [`assets/adapter-board-demo.jpg`](./assets/adapter-board-demo.jpg) |

<p align="center">
  <img alt="3.19&quot; AMOLED PCB outline (front)" src="./images/dimensions-front.png" width="280" />
  &nbsp;&nbsp;
  <img alt="3.19&quot; AMOLED PCB outline (back)" src="./images/dimensions-back.png" width="280" />
</p>

### Samples

- [ESP32-P4 CO6300 MIPI + CST3530 + LVGL](./examples/esp32p4-amoled-3.19-262x928-mipi-co6300-bringup/)

## Where to Buy

<p align="center">
  <a href="https://www.aliexpress.com/store/1105701619"><img alt="AliExpress Official Store" src="https://img.shields.io/badge/AliExpress-Official_Store-E62E04?style=for-the-badge&logo=aliexpress&logoColor=white" /></a>
  &nbsp;&nbsp;
  <a href="https://shop110742373.taobao.com/"><img alt="Taobao Official Store" src="https://img.shields.io/badge/Taobao-Official_Store-FF6A00?style=for-the-badge" /></a>
</p>

**International (AliExpress)**

- Store: [OSPTEK Official Store](https://www.aliexpress.com/store/1105701619)

**China (Taobao)**

- Store: [鱼鹰光电工厂店](https://shop110742373.taobao.com/)

## Support

- Technical Support / Sales: <luyu@osptek.com>
- QQ Technical Group: **985881096**
- Website: <https://osptek.com/>
- Feel free to open an Issue in this repository if you have any questions

---

<p align="center"><sub>© 2026 OSPTEK · Licensed under CC BY 4.0</sub></p>

# StandWithUkraine

[![SWUbanner](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/banner-direct.svg)](https://github.com/vshymanskyy/StandWithUkraine/blob/main/docs/README.md)


# AlertLight - ESP32-S3 Air Alert & Power Outage Monitor

![ESP32-S3](https://img.shields.io/badge/ESP32-S3-blue)
![LVGL](https://img.shields.io/badge/LVGL-8.x-green)
![Status](https://img.shields.io/badge/Status-Production-success)
![License](https://img.shields.io/badge/License-MIT-yellow)
[![StandWithUkraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/badges/StandWithUkraine.svg)](https://github.com/vshymanskyy/StandWithUkraine/blob/main/docs/README.md)

A smart monitoring device for Ukrainian air alerts and power outage schedules, featuring a 1.47" color display and RGB LED notifications.

## 📋 Overview

AlertLight is an ESP32-S3 based monitoring device that:
- Displays real-time air alert status for Ukrainian regions (AJAX system)
- Shows planned power outage schedules (Yasno API)
- Provides RGB LED notifications for status changes
- Offers comprehensive web-based configuration
- Runs continuously with automatic updates

## 🎯 Key Features

### Display (1.47" ST7789 172×320)
- **Three-screen system**:
  - Init screen with boot animation
  - Boot log screen with system startup messages
  - Main status screen with live data
- **Status sections**:
  - Device network information (IP, WiFi status)
  - Air alert status for selected region
  - Power outage schedule with time slots

### RGB LED Notifications (WS2812B)
- **Ambient status indication**:
  - 🟢 Green: No alert, no outage
  - 🔴 Red: Air alert active
  - 🔵 Blue: Power outage in progress
  - ⚪ Grey: No connection/unknown status
- **Event notifications** (30-second blinks):
  - 🔴 Red blink: Air alert started
  - 🟢 Green blink: Air alert dismissed
  - 🔵 Blue blink: Power outage started
  - 🟡 Yellow blink: Power restored
- **Priority system**: Alert > Outage > Normal
- **Fully customizable** colors and timing via web interface

### Web Configuration Interface
Access via device IP address (default port 8080):

- **📡 WiFi Settings**: SSID, password, static IP configuration
- **🚨 Alert Settings**: Region selection, check interval, API configuration
- **⚡ Power Outage Settings**: Queue selection, DSO configuration
- **🌈 RGB Settings**: Colors, brightness, blink patterns, test controls
- **📺 Display Settings**: Brightness adjustment
- **⏰ NTP Time**: Manual time synchronization
- **📊 Status Page**: Live monitoring data, last check times, HTTP responses
- **🔄 System**: Factory reset, device restart

## 🔧 Hardware Requirements

- **MCU**: ESP32-S3 (QFN56, 8MB PSRAM)
- **Display**: 1.47" ST7789 (172×320 SPI)
- **RGB LED**: WS2812B (NeoPixel compatible)
- **WiFi**: 2.4GHz 802.11 b/g/n
- **Storage**: 16MB Flash (3MB APP / 9.9MB FATFS partition)
- **Power**: USB-C or 5V external

## 📦 Software Architecture

### Core Components

```
AlertLight/
├── AlertLight.ino              # Main sketch
├── src/
│   ├── AlertLight_UI/          # LVGL UI screens and rendering
│   ├── AlertManager/           # Air alert API integration
│   ├── LightManager/           # Power outage API integration
│   ├── RGBManager/             # RGB LED control and notifications
│   ├── WebConfig/              # Web server and configuration
│   ├── Config/                 # NVS configuration storage
│   ├── RegionMapper/           # Ukrainian region ID mapping
│   ├── LVGL_Driver/            # Display driver integration
│   └── RGB_Lamp/               # WS2812B LED driver
└── UI_Mockup/                  # EEZ Studio UI design files
```

### Key Managers

- **AlertManager**: Monitors AJAX air alert API, detects state changes
- **LightManager**: Parses Yasno power outage schedules, tracks current status
- **RGBManager**: Controls LED colors, handles event notifications with priority
- **WebConfigManager**: Serves configuration interface, handles API endpoints
- **ConfigManager**: Manages persistent settings in NVS flash

## 🚀 Getting Started

### 🌐 Quick Install (Web Installer - Recommended)

**No software installation needed! Flash firmware directly from your browser.**

#### Requirements:
- Chrome, Edge, or Opera browser (Web Serial API support)
- ESP32-S3 device connected via USB-C
- Internet connection

#### Steps:

**Option 1: Online Web Installer (Easiest)**
1. Visit: **[https://coolerUA.github.io/AlertLight/install.html](https://coolerUA.github.io/AlertLight/install.html)**
2. Connect your ESP32-S3 device via USB
3. Click **"Install AlertLight"** button
4. Select the correct serial port when prompted
5. Wait for installation to complete (~2 minutes)
6. Done! Device will restart automatically

**Option 2: Local Web Installer**
1. Download the latest release from [GitHub Releases](https://github.com/coolerUA/AlertLight/releases)
2. Extract the files
3. Open `install.html` in Chrome/Edge browser
4. Follow the on-screen instructions

#### After Installation:
- Device creates WiFi AP: `AlertLight-Setup-[MAC]`
- Connect to AP and go to `http://192.168.4.1:8080`
- Configure your WiFi and preferences
- Enjoy! 🎉

---

### 💻 Advanced Installation (Arduino IDE)

**For developers or those who want to customize the firmware.**

#### Prerequisites

1. **Arduino IDE** or **PlatformIO**
2. **ESP32 Board Package** (v3.0+)
3. **Required Libraries**:
   - LVGL 8.x
   - ArduinoJson
   - WiFi (built-in)
   - HTTPClient (built-in)
   - Preferences (built-in)
   - Adafruit_NeoPixel

#### Installation Steps

1. Clone this repository:
   ```bash
   git clone https://github.com/coolerUA/AlertLight
   cd AlertLight
   ```

2. Install required libraries via Arduino Library Manager

3. Configure board settings:
   - Board: ESP32-S3 Dev Module
   - Flash Size: 16MB
   - Partition Scheme: 16M Flash (3MB APP/9.9MB FATFS)
   - PSRAM: Enabled (OPI PSRAM)
   - USB CDC On Boot: Enabled
   - Upload Speed: 921600

4. Compile and upload

### First Time Setup

1. Power on the device
2. Device will create WiFi AP: `AlertLight-Setup-[MAC]`
3. Connect to AP and navigate to `http://192.168.4.1:8080`
4. Configure WiFi credentials
5. Device will restart and connect to your network
6. Access web interface at device IP address

## 🌐 Web Interface Guide

### WiFi Configuration
- **SSID**: Your WiFi network name
- **Password**: WiFi password
- **Static IP**: Optional manual IP configuration
- **Port**: Web interface port (default: 8080)

### Alert Configuration
- **Region**: Select your Ukrainian region (Kyiv, Lviv, Odesa, etc.)
- **API URL**: Air alert API endpoint (pre-configured)
- **Check Interval**: How often to check for alerts (default: 30 seconds)

### Light Outage Configuration
- **Queue**: Your power outage queue (e.g., "6.2")
- **API URL**: Yasno API endpoint (pre-configured)
- **Check Interval**: Schedule update frequency (default: 15 minutes)

### RGB Configuration
- **Ambient Brightness**: LED brightness during normal operation (0-100%)
- **Status Colors**: RGB values for each state (no alert, alert, outage, no status)
- **Blink Colors**: RGB values for event notifications
- **Blink Timing**: ON duration, OFF duration, total duration
- **Test Controls**: Real-time testing of all RGB modes

## 📡 API Integration

### Air Alert API (AJAX)
- **Endpoint**: `https://air-save.ops.ajax.systems/api/mobile/status/regions/v2`
- **Method**: GET with region parameter
- **Update**: Every 30 seconds (configurable)
- **Regions**: Supports all 27 Ukrainian administrative regions

### Power Outage API (Yasno)
- **Endpoint**: `https://app.yasno.ua/api/blackout-service/public/shutdowns`
- **Method**: GET with region/DSO/queue parameters
- **Update**: Every 15 minutes (configurable)
- **Features**: Displays today's outage schedule with time slots

## 🎨 Display Sections

### Section 1: Network Info
- Current IP address and port
- WiFi connection status icon
- Blinks when in AP mode

### Section 2: Air Alert Status
- Selected region name
- Current alert status
- Last API check time
- HTTP response code

### Section 3: Power Outage Schedule
- Queue identifier
- Today's outage time slots (up to 3 shown)
- Active slot highlighting
- Last update timestamp

## 🔔 RGB Notification System

### Event Detection
The system automatically detects and notifies on:

1. **Alert Started**: API shows alert for configured region → Red blink (30s)
2. **Alert Dismissed**: Alert clears for region → Green blink (30s)
3. **Outage Started**: Current time enters outage slot → Blue blink (30s)
4. **Power Restored**: Current time exits outage slot → Yellow blink (30s)

### Priority System
If multiple events occur simultaneously:
- Alert Start (highest priority)
- Alert Dismiss
- Outage Start
- Power Restore
- Ambient (lowest priority)

Higher priority events interrupt lower priority blinks.

## 🛠️ Troubleshooting

### Device won't connect to WiFi
1. Check SSID and password in web config
2. Ensure 2.4GHz WiFi (5GHz not supported)
3. Check signal strength
4. Try factory reset via web interface

### RGB LED not working
1. Verify WS2812B wiring (DIN pin)
2. Check RGB test controls in web interface
3. Ensure 5V power supply is adequate
4. Check GRB color order setting

### Display shows wrong data
1. Check API URLs in configuration
2. Verify internet connectivity
3. Check NTP time synchronization
4. Review serial monitor for API errors

### Web interface not accessible
1. Find device IP from serial monitor
2. Check device is on same network
3. Try default port 8080
4. Reset network settings if needed

## 📊 Serial Monitor Output

Enable serial monitor (115200 baud) to see:
- Boot sequence and initialization
- WiFi connection status
- API request/response data
- Alert state changes
- RGB event triggers
- Error messages and diagnostics

## 🔐 Security Notes

- WiFi passwords stored in NVS flash (not encrypted)
- Web interface has no authentication (local network only)
- HTTPS used for external API calls
- Consider network isolation for production use

## 📝 Configuration Files

All settings stored in ESP32 NVS (Non-Volatile Storage):
- Persists across reboots
- Factory reset available via web interface
- No external SD card required for config

## 🤝 Contributing

This project is specific to Ukrainian infrastructure monitoring. Contributions welcome for:
- Bug fixes
- UI improvements
- Additional region support
- Localization
- Documentation

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Copyright (c) 2025 [coolerUA](https://github.com/coolerUA)

## 🙏 Acknowledgments

- **AJAX Systems**: Air alert API
- **Yasno/DTEK**: Power outage schedule API
- **LVGL**: Graphics library
- **ESP32 Community**: Arduino framework and libraries

## 📞 Support

For issues and questions:
- Check `PROJECT_REQUIREMENTS.md` for detailed specifications
- Review serial monitor output for errors
- Check web interface status page for API responses

## 🔄 Version History

### v1.0.0 (Current)
- ✅ Complete implementation of all core features
- ✅ Three-screen LVGL UI
- ✅ Dual API integration (Alert + Power Outage)
- ✅ RGB notification system with priority
- ✅ Comprehensive web configuration
- ✅ NVS persistent storage
- ✅ NTP time synchronization
- ✅ WiFi manager with AP fallback
- ✅ State change detection and event triggering

---

**Made with ❤️ for Ukraine 🇺🇦**

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

AlertLight is an ESP32-S3 based monitoring device for Ukrainian air alerts (AJAX system) and power outage schedules (Yasno API). It features a 1.47" ST7789 color display (172×320), WS2812B RGB LED notifications, and a comprehensive web-based configuration interface.

**Hardware:** ESP32-S3 (QFN56, 8MB PSRAM, 16MB Flash), ST7789 SPI display, WS2812B RGB LED

**Status:** Production ready (v1.0.1)

## Build System

This project uses **Arduino CLI** (not PlatformIO).

### Compilation

```bash
# Compile the project
arduino-cli compile --fqbn esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi --build-property "build.partitions=app3M_fat9M_16MB" --build-property "build.defines=-DBOARD_HAS_PSRAM" .

# Upload to device
arduino-cli upload --fqbn esp32:esp32:esp32s3 -p /dev/ttyACM0 .

# Combined compile and upload
arduino-cli compile --fqbn esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi --build-property "build.partitions=app3M_fat9M_16MB" --build-property "build.defines=-DBOARD_HAS_PSRAM" --upload -p /dev/ttyACM0 .

# Monitor serial output
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

### Board Configuration

- **Board:** ESP32-S3 Dev Module (`esp32:esp32:esp32s3`)
- **Flash Size:** 16MB (128Mb)
- **Partition Scheme:** 16M Flash (3MB APP / 9.9MB FATFS) - `app3M_fat9M_16MB`
- **PSRAM:** OPI PSRAM (8MB) - **REQUIRED**
- **USB CDC On Boot:** Enabled (recommended for debugging)
- **Upload Speed:** 921600

### Required Libraries

Install via Arduino CLI:
```bash
arduino-cli lib install "LVGL"
arduino-cli lib install "ArduinoJson"
arduino-cli lib install "Adafruit NeoPixel"
```

Built-in libraries used: WiFi, HTTPClient, Preferences, WebServer, DNSServer

## Architecture

### Core Manager System

The project uses a manager-based architecture where each manager handles a specific domain:

1. **ConfigManager** (`src/Config/`) - NVS persistent storage for all settings
2. **WebConfigManager** (`src/WebConfig/`) - Web server, configuration interface, WiFi management
3. **AlertManager** (`src/AlertManager/`) - AJAX air alert API integration and state tracking
4. **LightManager** (`src/LightManager/`) - Yasno power outage API and schedule parsing
5. **RGBManager** (`src/RGBManager/`) - RGB LED control with priority-based event notifications
6. **AlertLight_UI** (`src/AlertLight_UI/`) - LVGL UI with 3-screen system

### Manager Interaction Flow

```
main loop → webConfig.handleClient() → HTTP requests
         → alertManager.update() → checks API → detects state changes → triggers rgbManager
         → lightManager.update() → checks API → parses schedule → updates UI
         → rgbManager.update() → processes event queue → controls RGB LED
         → AlertLight_UI_Tick() → updates display elements
```

### Key Design Patterns

- **Singleton managers:** Each manager is a global singleton instance
- **Pull-based updates:** Managers poll APIs at configurable intervals
- **Event-driven notifications:** State changes trigger RGB LED events via priority queue
- **Persistent configuration:** All settings stored in NVS, survive reboots

### WiFi Connection Architecture

**WebConfig.cpp:115-271** - `connectToWiFi()` function:
- Disconnects cleanly before new connection attempts
- Scans for networks with extended timeout (2000ms per channel) for dense WiFi environments
- Attempts connection even if SSID not found in scan (supports hidden networks)
- Supports both DHCP and static IP configuration
- Has comprehensive error handling with status codes
- Cleans up WiFi state on failure (disconnect + delay)

**WiFi Power Management** (CRITICAL for reliability):
- `WiFi.setSleep(false)` - Disables WiFi sleep mode for better scanning/connection
- `WiFi.setTxPower(WIFI_POWER_19_5dBm)` - Maximum TX power for better signal
- `esp_wifi_set_ps(WIFI_PS_NONE)` - Disables power saving for maximum performance
- `show_hidden=true` in scan parameters - Finds hidden SSIDs

**AP Fallback Mode:**
- If connection fails, device spawns AP: `AlertLight-[MAC]`
- AP IP: 192.168.4.1:8080 (web interface accessible)
- Periodic reconnection attempts every 60 seconds while in AP mode
- Automatically restarts AP mode if it stops during failed reconnection attempt

### RGB Notification System

**Priority-based event system** (`src/RGBManager/RGBManager.cpp:150-216`):

**Event Types** (highest to lowest priority):
1. `RGB_EVENT_ALERT_START` - Air alert started (red blink, 30s)
2. `RGB_EVENT_ALERT_DISMISS` - Air alert dismissed (green blink, 30s)
3. `RGB_EVENT_OUTAGE_START` - Power outage started (blue blink, 30s)
4. `RGB_EVENT_POWER_RESTORE` - Power restored (yellow blink, 30s)

**State Machine:**
- `RGB_STATE_AMBIENT` - Continuous status indication (dim)
- `RGB_STATE_BLINKING` - Active event notification (30s blink)

Higher priority events interrupt lower priority blinks. After blink completes, returns to ambient status color.

### LVGL UI Architecture

**Three-screen system:**
1. **Init Screen** (0-5s) - Boot animation with logo
2. **Boot Log Screen** (5-15s) - System startup messages, scrolling log
3. **Main Screen** (15s+) - Live status with 3 sections

**Main Screen Sections:**
- **Section 1:** IP address, port, WiFi status icon (connected/AP/disconnected)
- **Section 2:** Air alert status for selected region
- **Section 3:** Power outage schedule with time slots (highlights active slot)

**UI Update Functions** (`src/AlertLight_UI/AlertLight_UI.h:24-33`):
- `AlertLight_UI_Update_IP()` - Updates IP/port display
- `AlertLight_UI_Update_WiFi()` - Updates WiFi status icon
- `AlertLight_UI_Update_Alert()` - Updates alert section
- `AlertLight_UI_Update_Light()` - Updates power outage section
- `AlertLight_UI_Update_Clock()` - Updates time display (called every 1s)

## API Integration

### Air Alert API (AJAX)
- **Endpoint:** `https://air-save.ops.ajax.systems/api/mobile/status/regions/v2?regions={regionID}`
- **Update:** Every 30 seconds (configurable: `alert_check_interval`)
- **Response:** JSON with `alarms` array
- **Regions:** All 27 Ukrainian administrative regions (see `src/RegionMapper/`)

### Power Outage API (Yasno)
- **Endpoint:** `https://app.yasno.ua/api/blackout-service/public/shutdowns/regions/{regionID}/dsos/{dsoID}/planned-outages`
- **Update:** Every 15 minutes (configurable: `light_check_interval`)
- **Response:** JSON with queue-based schedules
- **Time Slots:** Minutes since midnight (e.g., 300 = 05:00, 1050 = 17:30)

### State Change Detection

**AlertManager** (`src/AlertManager/AlertManager.cpp:98-143`):
- Compares previous and current API responses
- Detects alert start/dismiss events
- Triggers RGB notifications via `rgbManager.triggerEvent()`

**LightManager** (`src/LightManager/LightManager.cpp:176-230`):
- Parses outage slots with `type: "Definite"` (confirmed outages)
- Compares current time against slot boundaries
- Detects outage start/restore transitions
- Updates UI with active slot highlighting

## Configuration System

**All settings stored in NVS** (`src/Config/Config.h:11-47`):

```cpp
struct AlertLightConfig {
    // WiFi
    char wifi_ssid[64];
    char wifi_password[64];
    bool use_static_ip;
    char static_ip[16], static_gateway[16], static_subnet[16];
    uint16_t web_port;

    // Alert
    char alert_region[32];
    uint16_t alert_region_id;
    char alert_api_url[256];
    uint32_t alert_check_interval;

    // Light
    char light_queue[16];
    uint16_t light_region_id, light_dso_id;
    char light_api_url[256];
    uint32_t light_check_interval;

    // RGB (ambient colors, blink colors, timing)
    uint8_t rgb_ambient_brightness;
    // ... many more RGB settings

    // Display
    uint8_t display_brightness;
};
```

**Configuration API:**
- `configManager.begin()` - Initialize NVS
- `configManager.getConfig()` - Returns reference to current config
- `configManager.saveConfig()` - Persists changes to NVS
- `configManager.resetToDefaults()` - Factory reset

## Common Development Tasks

### Modifying WiFi Connection Logic

WiFi connection happens in `src/WebConfig/WebConfig.cpp:115-256`. Key considerations:
- WiFi power settings are critical for reliability (sleep mode, TX power, power saving)
- Scan with `show_hidden=true` to find hidden SSIDs
- Extended scan time (2000ms) for dense WiFi environments
- Always verify SSID exists in scan results before attempting connection
- Clean disconnect (`WiFi.disconnect(true)`) before new attempts

### Adding New Configuration Parameters

1. Add field to `AlertLightConfig` struct in `src/Config/Config.h`
2. Update `resetToDefaults()` in `src/Config/Config.cpp` with default value
3. Add web form field in appropriate page (e.g., `handleWiFiConfig()` in `src/WebConfig/WebConfig.cpp`)
4. Add form handler to parse and save new parameter
5. Use `configManager.getConfig().your_param` to access value

### Adding New RGB Event Types

1. Add event enum in `src/RGBManager/RGBManager.h` (e.g., `RGB_EVENT_NEW_EVENT`)
2. Add priority in `RGBManager.cpp:150-163` `processEventQueue()`
3. Define colors in config struct or hardcode in `RGBManager::update()`
4. Trigger event: `rgbManager.triggerEvent(RGB_EVENT_NEW_EVENT)`

### Debugging WiFi Issues

Enable serial monitor (115200 baud) and check:
- Boot sequence shows "WiFi manager started"
- Scan results: "Found X networks"
- Target SSID found: ">>> Found: [SSID]"
- Connection status codes: WL_CONNECTED (3), WL_NO_SSID_AVAIL (1), WL_CONNECT_FAILED (4)
- IP acquisition: "WiFi connected - IP: X.X.X.X"

Common issues:
- WiFi sleep mode enabled → Disable with `WiFi.setSleep(false)`
- Hidden SSID not found → Enable `show_hidden=true` in scan
- Network beyond 50 in scan results → Removed limit, now scans all networks
- Weak signal → Check `WiFi.setTxPower()` set to maximum

### Debugging API Integration

Check web interface Status page (`http://[device-ip]:8080/status`) for:
- Last API check times
- HTTP response codes (200 = success)
- Raw API response data
- Last state change timestamps

Serial monitor shows:
- API request URLs
- JSON parsing results
- State change detection: "🚨 ALERT EVENT: Started"
- RGB event triggers

## File Structure Reference

```
AlertLight/
├── AlertLight.ino              # Main sketch with setup() and loop()
├── lv_conf.h                   # LVGL configuration (required in root)
├── build_opt.h                 # Compiler optimization flags
├── src/
│   ├── Config/                 # NVS configuration management
│   ├── WebConfig/              # Web server, WiFi manager, HTTP handlers
│   ├── AlertManager/           # Air alert API integration
│   ├── LightManager/           # Power outage API integration
│   ├── RGBManager/             # RGB LED control and event system
│   ├── AlertLight_UI/          # LVGL UI screens and rendering
│   ├── RegionMapper/           # Ukrainian region ID mapping
│   ├── Display/                # ST7789 display driver
│   ├── LVGL_Driver/            # LVGL integration layer
│   ├── RGB_Lamp/               # WS2812B low-level driver
│   ├── Wireless/               # WiFi scan diagnostics
│   └── SD_Card/                # SD card utilities (not used in production)
├── UI_Mockup/                  # EEZ Studio UI design files
├── install.html                # Web-based firmware installer
└── manifest.json               # ESP Web Tools manifest
```

## Important Notes

- **Flash configuration:** 16MB (128Mb) with partition scheme: 3MB APP / 9.9MB FATFS
- **PSRAM is mandatory** - LVGL display buffers require 8MB PSRAM
- **Serial monitor baud rate:** 115200
- **Web interface default port:** 8080 (configurable)
- **First boot:** Device creates AP `AlertLight-[MAC]` for configuration
- **Factory reset:** Available via web interface `/system` page
- **Time sync:** NTP used for accurate outage schedule tracking
- **RGB color order:** GRB (WS2812B standard)
- **Display orientation:** Portrait 172×320, 16-bit color

## Known Issues

- **WiFi connection fails with status 6 (Disconnected)** - This typically means authentication failed or signal is too weak during connection attempt (not during scan). Check:
  - Password is correct
  - Network is 2.4GHz (5GHz not supported)
  - Signal strength (RSSI) during scan vs. during connection attempt
  - WiFi power management is disabled
  - Try increasing connection timeout in `connectToWiFi(timeout_ms)`

- **Display shows "Disconnected" briefly on boot** - Normal behavior during initial WiFi connection. Boot screen shows "Connecting..." for 10 seconds.

## Hidden Network Support

The device supports hidden WiFi networks (SSIDs not broadcast):
- Network scan will report "SSID not found in scan (may be hidden network)"
- Connection attempt proceeds regardless of scan results
- Enter SSID exactly as configured in router (case-sensitive)
- Ensure password is correct - authentication failures are more common with hidden networks

## Testing & Validation

**RGB LED Testing:**
- Web interface has real-time test controls (`/rgb` page)
- Test all 4 event types and ambient modes
- Verify priority system: Start alert test while other blink is active

**API Testing:**
- Status page shows raw API responses
- Force update: Change check interval to 10 seconds temporarily
- Verify state changes trigger correct RGB events

**WiFi Testing:**
- Test DHCP and static IP modes
- Verify AP fallback when credentials are invalid
- Check reconnection logic by disconnecting router

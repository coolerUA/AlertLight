# AlertLight Project Requirements

## Project Overview
Transform the ESP32-S3 display example into an alert monitoring device with web configuration, displaying alert status and power outage schedules with RGB LED indicators.

---

## 1. Network & Web Interface

### 1.1 WiFi Connection Management
- **Primary Mode**: Connect to configured WiFi network
- **Fallback Mode**: If no configured WiFi or connection fails, spawn self-hosted Access Point (AP)
- AP should have recognizable SSID (e.g., "AlertLight-Setup-[MAC]")

### 1.2 Web Interface
- **Purpose**: Device configuration interface
- **Access**: Served on device IP address (configurable port)
- **Requirements**:
  - Responsive design for mobile/desktop
  - Configuration form for all user-settable parameters
  - Save/Apply configuration functionality
  - Display current device status

---

## 2. Display Layout (172×320, 1.47" ST7789)

### Display Structure
Split display into **3 horizontal sections**:

```
┌─────────────────────────┐
│  DEVICE ADDRESS         │ ← Section 1: Top
│  IP: 192.168.1.100:80   │
│  [WiFi Icon]            │
├─────────────────────────┤
│  ALERT SECTION          │ ← Section 2: Middle
│  ● Kyiv Region          │
│  Status: [Indicator]    │
├─────────────────────────┤
│  LIGHT SECTION          │ ← Section 3: Bottom
│  Queue: 1.2             │
│  Outages: 14:00-17:00   │
└─────────────────────────┘
```

---

## 3. Section 1: Device Address

### Display Elements
- **IP Address**: Device's current IP address
- **Port**: Web interface port number
- **WiFi Status Icon**:
  - Show WiFi symbol when connected to configured network
  - Show AP icon when in Access Point mode
  - Hide or show disconnected state when not connected

### Example Display
```
192.168.1.100:80
[📶 WiFi]
```

---

## 4. Section 2: Alert Status

### Data Source
- **API**: HTTPS requests to remote API
- **Update Frequency**: Configurable (default: every 5 minutes)
- **Parse**: JSON/XML response for alert status

### Display Elements
- **Region Name**: User-configured region identifier
- **Status Indicator**: Colored circle/icon showing current status

### Status States & Colors
| Status      | Color | Meaning                    |
|-------------|-------|----------------------------|
| No Status   | Grey  | API not responding/unknown |
| No Alert    | Green | Normal, no alerts          |
| Alert       | Red   | Alert active in region     |

### Configuration Parameters
- `regionID`: Region identifier for API requests
- `alertApiUrl`: Remote API endpoint
- `alertCheckInterval`: Update frequency (seconds)

---

## 5. Section 3: Light Outages

### Data Source
- **API**: HTTPS requests to remote API
- **Update Frequency**: Configurable (default: every 15 minutes)
- **Parse**: Schedule data for power outages

### Display Elements
- **Queue Name**: Light outage queue identifier (e.g., "1.2", "3.1")
- **Time Ranges**: Current/upcoming outage schedule
  - Format: `HH:MM-HH:MM`
  - Show multiple ranges if applicable
  - Show "No outages" if schedule is clear

### Example Display
```
Queue: 1.2
Today: 14:00-17:00
       20:00-23:00
```

### Configuration Parameters
- `lightQueue`: Queue identifier for API requests
- `lightApiUrl`: Remote API endpoint
- `lightCheckInterval`: Update frequency (seconds)

---

## 6. RGB Backlight Behavior

### Default State (Ambient)
- **Color**: Matches current status
- **Brightness**: Very low (configurable, default: 5-10%)

### Status Colors (Ambient)
| Condition                        | Color       |
|----------------------------------|-------------|
| No alert + No outage             | Light Green |
| Alert active                     | Red (dim)   |
| Outage in progress               | Dark Blue   |
| Gray (no status)                 | Gray/Off    |

### Alert Notifications (Blinks)
Blink for **30 seconds** when conditions are first detected:

| Trigger                                    | Blink Color | Duration |
|--------------------------------------------|-------------|----------|
| Alert appeared                             | Red         | 30s      |
| Power outage scheduled <30 min             | Dark Blue   | 30s      |
| Power restoration planned <30 min          | Yellow      | 30s      |

### Blink Pattern
- **ON time**: Configurable (default: 500ms)
- **OFF time**: Configurable (default: 500ms)
- **Total duration**: Configurable (default: 30s)

### Priority
If multiple conditions trigger:
1. Alert (highest priority - red)
2. Outage imminent (dark blue)
3. Restoration imminent (yellow)
4. Normal state (light green)

---

## 7. Web Interface Configuration

### Configurable Parameters

#### Network Settings
- WiFi SSID
- WiFi Password
- Static IP / DHCP toggle
- Web interface port

#### Alert Configuration
- Region ID for monitoring
- Alert API URL
- Alert check interval (seconds)
- Alert indicator colors (RGB values)

#### Light Outage Configuration
- Light queue identifier
- Light API URL
- Light check interval (seconds)

#### RGB LED Configuration
- **Ambient brightness** (0-100%)
- **Status colors** (RGB values for each state):
  - No alert color
  - Alert color
  - Outage color
  - No status color
- **Blink settings**:
  - Blink ON duration (ms)
  - Blink OFF duration (ms)
  - Total blink duration (seconds)
  - Alert blink color
  - Outage imminent blink color
  - Restoration imminent blink color

#### Display Settings
- Screen brightness
- Font sizes
- Update intervals

---

## 8. API Integration Requirements

### Alert API
**Expected Response Format** (example):
with alert
```json
{"alarms":[{"alarmType":"AIR","regionId":16,"customAlarmData":null,"rapidAlarmData":null}]}
```
without alert
```json
{"alarms":[]}
```

### Light Outage API
**Expected Response Format** (example):
```json
{
  "6.2": {
        "today": {
            "slots": [
                {
                    "start": 0,
                    "end": 300,
                    "type": "NotPlanned"
                },
                {
                    "start": 300,
                    "end": 390,
                    "type": "Definite"
                },
                {
                    "start": 390,
                    "end": 810,
                    "type": "NotPlanned"
                },
                {
                    "start": 810,
                    "end": 1050,
                    "type": "Definite"
                },
                {
                    "start": 1050,
                    "end": 1440,
                    "type": "NotPlanned"
                }
            ],
            "date": "2025-12-26T00:00:00+02:00",
            "status": "ScheduleApplies"
        },
        "tomorrow": {
            "slots": [
                {
                    "start": 0,
                    "end": 360,
                    "type": "NotPlanned"
                },
                {
                    "start": 360,
                    "end": 600,
                    "type": "Definite"
                },
                {
                    "start": 600,
                    "end": 1440,
                    "type": "NotPlanned"
                }
            ],
            "date": "2025-12-27T00:00:00+02:00",
            "status": "WaitingForSchedule"
        },
        "updatedOn": "2025-12-26T10:05:25+00:00"
    }
}
```

### Requirements
- HTTPS support (SSL/TLS)
- Handle network errors gracefully
- Timeout configuration
- Retry logic on failure
- Cache last known good data

---

## 9. Data Persistence

### Configuration Storage
Store in non-volatile memory (SPIFFS/LittleFS/Preferences):
- WiFi credentials
- API endpoints and credentials
- Region ID and queue number
- All color and timing configurations
- Last known IP address

### Runtime Data
- Current alert status
- Current outage schedule
- Last API update timestamps
- Connection status

---

## 10. Implementation Phases

### Phase 1: Basic Infrastructure ✅ COMPLETED
- [x] Hardware initialization (Display, RGB, WiFi)
- [x] WiFi connection manager with AP fallback
- [x] Basic web server setup
- [x] Configuration storage system (NVS Preferences)

### Phase 2: Display Layout ✅ COMPLETED
- [x] Create 3-screen LVGL layout (init, boot, main)
- [x] Device address section with WiFi icon
- [x] Alert section with status indicator
- [x] Light section with schedule display
- [x] Screen transitions and lifecycle management

### Phase 3: API Integration ✅ COMPLETED
- [x] HTTPS client implementation
- [x] Alert API integration and parsing (AJAX air alert system)
- [x] Light outage API integration and parsing (Yasno API)
- [x] Periodic update timers
- [x] Error handling and retry logic
- [x] JSON parsing with ArduinoJson
- [x] Region mapping system

### Phase 4: RGB Logic ✅ COMPLETED
- [x] Ambient status coloring
- [x] Blink notification system (4 event types)
- [x] State change detection
- [x] Priority-based color selection
- [x] Configurable blink patterns
- [x] Test mode with web controls

### Phase 5: Web Interface ✅ COMPLETED
- [x] Configuration page HTML/CSS
- [x] Form handlers for all settings
- [x] Multiple configuration pages (WiFi, Alert, Light, RGB, Display)
- [x] Configuration save/load
- [x] Status display page with live data
- [x] RGB test controls interface
- [x] NTP time synchronization page
- [x] Factory reset functionality

### Phase 6: Polish & Testing ✅ MOSTLY COMPLETED
- [x] Input validation
- [x] Error handling improvements
- [x] Comprehensive debug logging
- [x] Documentation (README.md)
- [x] Boot logging system
- [ ] OTA update support (optional - not implemented)
- [x] Field testing in progress

---

## 11. Technical Considerations

### Memory Management
- ESP32-S3 has sufficient RAM, but monitor heap usage
- Use PSRAM if available for display buffers
- Implement JSON parsing with streaming if responses are large

### Power Consumption
- Consider sleep modes if battery-powered
- Adjust display refresh rate
- WiFi power management

### Security
- Store WiFi passwords encrypted
- Consider HTTPS for web interface
- API authentication tokens (if required)
- Input sanitization for web forms

### Time Synchronization
- Use NTP for accurate timestamps
- Handle timezone configuration
- Calculate "30 minutes from now" correctly

### User Experience
- Visual feedback during configuration
- Clear error messages on display
- Recovery mode if configuration corrupted
- Factory reset option

---

## 12. Current Project State vs. Requirements

### ✅ Fully Implemented
- ✅ Hardware initialization (Display, RGB, WiFi, SD card)
- ✅ Display driver (ST7789) with LVGL
- ✅ RGB LED support (WS2812B)
- ✅ WiFi connection manager with AP fallback mode
- ✅ Web server with comprehensive configuration interface
- ✅ 3-screen LVGL layout (init, boot log, main status)
- ✅ HTTPS API client with SSL/TLS support
- ✅ JSON parsing for API responses (ArduinoJson)
- ✅ RGB notification logic with priority system
- ✅ Configuration persistence (NVS)
- ✅ Time synchronization (NTP)
- ✅ Real-time status monitoring and display
- ✅ State change detection for events
- ✅ All configuration parameters accessible via web interface

### 🔧 Implemented Features

#### Core Functionality
- **WiFi Management**: Auto-connect, AP fallback, reconnection logic
- **Web Interface**: Multi-page configuration, status monitoring, RGB testing
- **Display**: Three-screen system with init, boot log, and live status
- **Alert Monitoring**: AJAX air alert system integration with region selection
- **Power Outage**: Yasno API integration with schedule display
- **RGB Notifications**: 4 event types (alert start/dismiss, outage start/restore)
- **Configuration**: Persistent storage with web-based management

#### API Integrations
- **Alert API**: `https://air-save.ops.ajax.systems/api/mobile/status/regions/v2`
- **Light API**: `https://app.yasno.ua/api/blackout-service/public/shutdowns`
- Region mapping for all Ukrainian regions
- Automatic periodic checks with configurable intervals

#### RGB LED Features
- Ambient status indication (green/red/blue/grey)
- Priority-based blink notifications (30 seconds)
- Full color customization via web interface
- Real-time test controls
- GRB color order support for WS2812B

### ⚠️ Known Limitations
- ❌ OTA update support not implemented
- ❌ Emergency shutdown detection (30-min threshold for outages) - partially implemented
- ❌ Battery operation / power management - not applicable (mains powered)
- ⚠️ Light outage "Emergency" status detection needs field verification

---

## 13. Actual API Endpoints

### Alert API (AJAX Air Alert System)
```cpp
// GET https://air-save.ops.ajax.systems/api/mobile/status/regions/v2?regions={regionID}
// Example: https://air-save.ops.ajax.systems/api/mobile/status/regions/v2?regions=16

// Response with alert:
{"alarms":[{"alarmType":"AIR","regionId":16,"customAlarmData":null,"rapidAlarmData":null}]}

// Response without alert:
{"alarms":[]}
```

**Supported Regions**: All Ukrainian administrative regions (see RegionMapper in src/RegionMapper/)

### Light Outage API (Yasno)
```cpp
// GET https://app.yasno.ua/api/blackout-service/public/shutdowns/regions/{regionID}/dsos/{dsoID}/planned-outages
// Example: https://app.yasno.ua/api/blackout-service/public/shutdowns/regions/25/dsos/902/planned-outages

// Response includes queue-based schedules with time slots
// Queue format: "6.2" - identifies the specific outage schedule
// Slot types: "Definite" (confirmed outage), "NotPlanned" (power on), "Emergency"
```

**Default Configuration**:
- Region: 25 (Kyiv)
- DSO: 902 (DTEK Kyiv Electric Networks)
- Queue: 6.2

---

## Notes
- ✅ All hardcoded values are now configurable via web interface
- ✅ Configuration stored in NVS with factory reset support
- ⚠️ Localization (Ukrainian/English) - currently mixed (UI in English, regions in Ukrainian)
- ✅ Boot logging system implemented (on-screen during startup)
- ✅ Flash size 16MB, Partition Scheme: 16M Flash (3MB APP / 9.9MB FATFS)
- ✅ Serial monitor provides comprehensive debug output

## Project Status: ✅ PRODUCTION READY

All major features have been implemented and tested. The device is fully functional with:
- Complete hardware integration (display, RGB LED, WiFi)
- Dual API monitoring (air alerts + power outages)
- Comprehensive web configuration interface
- RGB notification system with event detection
- Persistent configuration storage
- Automatic updates and state tracking

**Last Updated**: 2025-12-30
**Version**: 1.0.0
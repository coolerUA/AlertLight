# AlertLight Display UI Mockup

## Overview
This folder contains the UI mockup for the AlertLight display created in EEZ Studio format.

**Display Specifications:**
- Size: 1.47" ST7789
- Resolution: 172×320 pixels
- Color: RGB565 (262K colors)
- Orientation: Portrait

## File Structure
```
UI_Mockup/
├── AlertLight_Display.eez-project  # EEZ Studio project file
├── README.md                        # This file
└── mockup_visual.svg                # Visual representation (SVG)
```

## Design Layout

### Three-Section Design (No Section Labels)

```
┌────────────────────────────┐  ← 0px
│  192.168.1.100      [📶]   │  Top Section (50px)
│                            │  IP address + WiFi icon
├────────────────────────────┤  ← 50px
│      ALERT STATUS          │
│                            │  Alert Section (135px)
│         ⬤ LARGE            │  - Colored indicator circle
│                            │  - Region name
│        Kyiv                │  - Status text
│      No Alert              │
│                            │
├────────────────────────────┤  ← 185px
│    POWER OUTAGES           │
│                            │  Light Section (135px)
│  ┌─────────────────────┐  │  - Queue number
│  │ Queue:  6.2         │  │  - Today's schedule
│  └─────────────────────┘  │  - Time ranges
│                            │
│  Today:                    │
│  05:00-06:30               │
│  13:30-17:30               │
│                            │
└────────────────────────────┘  ← 320px
```

## Section Details

### 1. Top Section (Height: 50px)
**Minimalistic design with only essential info:**
- **IP Address**: Left-aligned, cyan color (#00aaff), 14px font
- **WiFi Icon**: Right-aligned, green when connected, hidden when disconnected
- **Background**: Dark gray (#1a1a1a)
- **Border**: Bottom border only (#333333)

### 2. Alert Section (Height: 135px)
**Alert monitoring with clear visual indicator:**
- **Title**: "ALERT STATUS" - small gray text, 11px
- **Indicator**: Large colored circle (40px diameter)
  - Grey (#808080) - No data
  - Green (#00ff00) - No alert
  - Red (#ff0000) - Alert active
- **Region Name**: Bold white text, 16px (e.g., "Kyiv")
- **Status Text**: Colored status message, 12px
- **Background**: Very dark (#0a0a0a)

### 3. Light Section (Height: 135px)
**Power outage schedule:**
- **Title**: "POWER OUTAGES" - small gray text, 11px
- **Queue Box**: Dark container with queue number
  - Label: "Queue:" in gray
  - Value: Queue number in orange (#ffaa00), bold, 14px
- **Schedule Title**: "Today:" in gray, 11px
- **Time Ranges**: Orange text (#ff6600), multi-line
  - Format: HH:MM-HH:MM
  - Multiple ranges on separate lines
- **Background**: Very dark (#0a0a0a)

## Color Palette

### Background Colors
- Main background: `#000000` (Black)
- Section backgrounds: `#0a0a0a` (Very dark gray)
- Top section: `#1a1a1a` (Dark gray)
- Queue container: `#1a1a1a` (Dark gray)
- Borders: `#333333` (Medium gray)

### Status Colors
- Alert Grey (No data): `#808080`
- Alert Green (Safe): `#00ff00`
- Alert Red (Active): `#ff0000`

### Text Colors
- IP Address: `#00aaff` (Cyan)
- WiFi Icon: `#00ff00` (Green)
- Primary text: `#ffffff` (White)
- Secondary labels: `#888888` / `#aaaaaa` (Gray)
- Queue number: `#ffaa00` (Orange)
- Outage times: `#ff6600` (Orange-red)

## Variables (Dynamic Data)

The mockup uses these global variables that will be updated from ESP32:

| Variable | Type | Default | Description |
|----------|------|---------|-------------|
| `ip_address` | string | "192.168.1.100" | Device IP address |
| `wifi_connected` | boolean | true | WiFi connection status |
| `region_name` | string | "Kyiv" | Alert region name |
| `alert_status` | integer | 1 | 0=grey, 1=green, 2=red |
| `light_queue` | string | "6.2" | Light outage queue |
| `outage_schedule` | string | "05:00-06:30\n13:30-17:30" | Outage time ranges |

## Usage with EEZ Studio

### Opening the Project
1. Install [EEZ Studio](https://github.com/eez-open/studio)
2. Open `AlertLight_Display.eez-project`
3. Edit the layout, colors, and styles as needed
4. Export LVGL code for ESP32 integration

### Exporting for LVGL
EEZ Studio can generate LVGL C code:
1. File → Build
2. Select "LVGL" target
3. Generate code
4. Copy generated files to ESP32 project

## Integration Notes

### LVGL Implementation
The mockup is designed for LVGL v8+. Key integration points:

```cpp
// Update variables from ESP32 code
lv_label_set_text(ip_label, WiFi.localIP().toString().c_str());
lv_obj_set_hidden(wifi_icon, !WiFi.isConnected());
lv_obj_set_style_bg_color(alert_indicator, lv_color_hex(color), 0);
```

### Dynamic Updates
- **IP Address**: Update on WiFi connection/DHCP lease
- **WiFi Icon**: Show/hide based on connection status
- **Alert Indicator**: Change color based on API response
- **Region Name**: Set from configuration
- **Outage Schedule**: Update when API data changes (every 15 min)

## Design Principles

1. **Minimalism**: No unnecessary labels or decorations
2. **Readability**: High contrast, appropriate font sizes
3. **At-a-glance**: Clear visual indicators (colors, icons)
4. **Information Hierarchy**: Most important info (alerts) is prominent
5. **Dark Theme**: Reduces power consumption, easier on eyes

## Future Enhancements

Possible UI improvements:
- [ ] Animated transitions between states
- [ ] Progress bar for next API update
- [ ] Tomorrow's schedule preview
- [ ] Last update timestamp
- [ ] Network signal strength indicator
- [ ] Battery status (if battery-powered)
- [ ] Configuration mode indicator

## Notes

- Icons (WiFi, etc.) need to be created as bitmaps
- Fonts should be embedded in the LVGL build
- Test on actual hardware for readability
- Adjust brightness levels for ambient lighting
- Consider screen burn-in prevention (pixel shift, screensaver)

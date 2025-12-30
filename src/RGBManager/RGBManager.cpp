#include "RGBManager.h"
#include "../AlertManager/AlertManager.h"
#include "../LightManager/LightManager.h"
#include "../RGB_Lamp/RGB_lamp.h"

RGBManager rgbManager;

RGBManager::RGBManager() {
    previousAlertState = false;
    previousOutageState = false;
    currentMode = MODE_AMBIENT;
    blinkStartTime = 0;
    lastBlinkToggle = 0;
    blinkState = false;
    testMode = false;
}

void RGBManager::begin() {
    printf("RGB Manager initialized\n");
    currentMode = MODE_AMBIENT;
}

void RGBManager::update() {
    // In test mode, don't auto-timeout blinks (user controls when to exit)
    if (!testMode) {
        // Check for blink timeout in normal mode
        if (currentMode != MODE_AMBIENT) {
            AlertLightConfig& cfg = configManager.getConfig();
            unsigned long blinkDuration = cfg.blink_total_duration * 1000; // Convert to ms

            if (millis() - blinkStartTime >= blinkDuration) {
                printf("Blink completed, returning to ambient mode\n");
                currentMode = MODE_AMBIENT;
            }
        }
    }

    // Update based on current mode (works in both test and normal mode)
    switch (currentMode) {
        case MODE_AMBIENT:
            updateAmbient();
            break;
        case MODE_BLINK_ALERT_START:
        case MODE_BLINK_ALERT_DISMISS:
        case MODE_BLINK_OUTAGE_START:
        case MODE_BLINK_RESTORE:
            updateBlink();
            break;
        case MODE_TEST:
            // Test mode with no active pattern - keep current color
            break;
    }
}

void RGBManager::notifyAlertStarted() {
    printf("[RGBManager] notifyAlertStarted() called, testMode=%d\n", testMode);
    if (testMode) {
        printf("[RGBManager] Blocked by test mode\n");
        return;
    }
    startBlink(MODE_BLINK_ALERT_START);
}

void RGBManager::notifyAlertDismissed() {
    printf("[RGBManager] notifyAlertDismissed() called, testMode=%d\n", testMode);
    if (testMode) {
        printf("[RGBManager] Blocked by test mode\n");
        return;
    }
    startBlink(MODE_BLINK_ALERT_DISMISS);
}

void RGBManager::notifyOutageStarted() {
    if (testMode) {
        return;
    }
    startBlink(MODE_BLINK_OUTAGE_START);
}

void RGBManager::notifyPowerRestored() {
    if (testMode) {
        return;
    }
    startBlink(MODE_BLINK_RESTORE);
}

void RGBManager::startBlink(RGBMode mode) {
    printf("[RGBManager] startBlink() called with mode=%d\n", mode);

    // Priority check - only interrupt if new event has higher priority
    RGBPriority newPriority;
    RGBPriority currentPriority;

    switch (mode) {
        case MODE_BLINK_ALERT_START: newPriority = PRIORITY_ALERT_START; break;
        case MODE_BLINK_ALERT_DISMISS: newPriority = PRIORITY_ALERT_DISMISS; break;
        case MODE_BLINK_OUTAGE_START: newPriority = PRIORITY_OUTAGE_START; break;
        case MODE_BLINK_RESTORE: newPriority = PRIORITY_RESTORE; break;
        default: return;
    }

    switch (currentMode) {
        case MODE_BLINK_ALERT_START: currentPriority = PRIORITY_ALERT_START; break;
        case MODE_BLINK_ALERT_DISMISS: currentPriority = PRIORITY_ALERT_DISMISS; break;
        case MODE_BLINK_OUTAGE_START: currentPriority = PRIORITY_OUTAGE_START; break;
        case MODE_BLINK_RESTORE: currentPriority = PRIORITY_RESTORE; break;
        default: currentPriority = PRIORITY_AMBIENT; break;
    }

    printf("[RGBManager] Priority check: new=%d, current=%d, currentMode=%d\n", newPriority, currentPriority, currentMode);

    if (newPriority >= currentPriority) {
        printf("[RGBManager] Starting blink! Setting mode to %d\n", mode);
        currentMode = mode;
        blinkStartTime = millis();
        lastBlinkToggle = millis();
        blinkState = true;
    } else {
        printf("[RGBManager] Blink blocked by higher priority current mode\n");
    }
}

void RGBManager::updateBlink() {
    AlertLightConfig& cfg = configManager.getConfig();
    unsigned long now = millis();
    unsigned long blinkInterval = blinkState ? cfg.blink_on_duration : cfg.blink_off_duration;

    if (now - lastBlinkToggle >= blinkInterval) {
        lastBlinkToggle = now;
        blinkState = !blinkState;
    }

    if (blinkState) {
        uint32_t color = getBlinkColor(currentMode);
        setRGBColor(color, 255); // Full brightness for blink
    } else {
        setRGBColor(0x000000, 0); // Off
    }
}

void RGBManager::updateAmbient() {
    uint32_t color;
    uint8_t brightness;
    getRGBAmbientColor(color, brightness);
    setRGBColor(color, brightness);
}

uint32_t RGBManager::getBlinkColor(RGBMode mode) {
    AlertLightConfig& cfg = configManager.getConfig();
    switch (mode) {
        case MODE_BLINK_ALERT_START:
            return cfg.color_blink_alert;
        case MODE_BLINK_ALERT_DISMISS:
            return cfg.color_blink_alert_dismiss;
        case MODE_BLINK_OUTAGE_START:
            return cfg.color_blink_outage;
        case MODE_BLINK_RESTORE:
            return cfg.color_blink_restore;
        default:
            return 0x000000;
    }
}

void RGBManager::getRGBAmbientColor(uint32_t& color, uint8_t& brightness) {
    AlertLightConfig& cfg = configManager.getConfig();
    brightness = map(cfg.ambient_brightness, 0, 100, 0, 255);

    // Priority: Alert > Outage > No Alert
    bool isAlert = alertManager.isAlertActive();
    bool isOutage = lightManager.isCurrentlyOutage();
    bool isEmergency = lightManager.isEmergencyShutdown();

    if (isAlert) {
        color = cfg.color_alert;
    } else if (isEmergency || isOutage) {
        color = cfg.color_outage;
    } else if (alertManager.getLastHTTPCode() == 200) {
        // Connected and no alert
        color = cfg.color_no_alert;
    } else {
        // No status (not connected to API)
        color = cfg.color_no_status;
    }
}

void RGBManager::setRGBColor(uint32_t color, uint8_t brightness) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    // Apply brightness
    r = (r * brightness) / 255;
    g = (g * brightness) / 255;
    b = (b * brightness) / 255;

    // WS2812B LEDs use GRB order, not RGB
    RGB_Lamp_SetColor(g, r, b);
}

uint32_t RGBManager::getCurrentColor() {
    if (currentMode == MODE_AMBIENT) {
        uint32_t color;
        uint8_t brightness;
        getRGBAmbientColor(color, brightness);
        return color;
    } else {
        return getBlinkColor(currentMode);
    }
}

bool RGBManager::isBlinking() {
    return currentMode != MODE_AMBIENT && currentMode != MODE_TEST;
}

String RGBManager::getCurrentMode() {
    switch (currentMode) {
        case MODE_AMBIENT: return "Ambient";
        case MODE_BLINK_ALERT_START: return "Alert Started (Red)";
        case MODE_BLINK_ALERT_DISMISS: return "Alert Dismissed (Green)";
        case MODE_BLINK_OUTAGE_START: return "Outage Started (Blue)";
        case MODE_BLINK_RESTORE: return "Power Restored (Yellow)";
        case MODE_TEST: return "Test Mode";
        default: return "Unknown";
    }
}

// Test mode functions
void RGBManager::setTestMode(bool enabled) {
    testMode = enabled;
    if (enabled) {
        printf("RGB Test Mode: ENABLED\n");
        currentMode = MODE_TEST;
    } else {
        printf("RGB Test Mode: DISABLED - returning to ambient\n");
        currentMode = MODE_AMBIENT;
    }
}

void RGBManager::testBlinkAlertStart() {
    if (!testMode) setTestMode(true);
    currentMode = MODE_BLINK_ALERT_START;
    blinkStartTime = millis();
    lastBlinkToggle = millis();
    blinkState = true;
    printf("TEST: Alert Start blink (RED)\n");
}

void RGBManager::testBlinkAlertDismiss() {
    if (!testMode) setTestMode(true);
    currentMode = MODE_BLINK_ALERT_DISMISS;
    blinkStartTime = millis();
    lastBlinkToggle = millis();
    blinkState = true;
    printf("TEST: Alert Dismiss blink (GREEN)\n");
}

void RGBManager::testBlinkOutageStart() {
    if (!testMode) setTestMode(true);
    currentMode = MODE_BLINK_OUTAGE_START;
    blinkStartTime = millis();
    lastBlinkToggle = millis();
    blinkState = true;
    printf("TEST: Outage Start blink (BLUE)\n");
}

void RGBManager::testBlinkRestore() {
    if (!testMode) setTestMode(true);
    currentMode = MODE_BLINK_RESTORE;
    blinkStartTime = millis();
    lastBlinkToggle = millis();
    blinkState = true;
    printf("TEST: Power Restore blink (YELLOW)\n");
}

void RGBManager::testAmbient() {
    if (!testMode) setTestMode(true);
    currentMode = MODE_AMBIENT;
    printf("TEST: Ambient mode\n");
}

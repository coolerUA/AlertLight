#ifndef RGBMANAGER_H
#define RGBMANAGER_H

#include <Arduino.h>
#include "../Config/Config.h"

// RGB state priorities (higher = more important)
enum RGBPriority {
    PRIORITY_AMBIENT = 0,      // Normal ambient status display
    PRIORITY_RESTORE = 1,      // Power restoration (yellow)
    PRIORITY_OUTAGE_START = 2, // Power outage starting (blue)
    PRIORITY_ALERT_DISMISS = 3,// Alert dismissed (green)
    PRIORITY_ALERT_START = 4   // Alert started (red) - highest priority
};

// RGB modes
enum RGBMode {
    MODE_AMBIENT,              // Show ambient status color
    MODE_BLINK_ALERT_START,    // Blink red - alert started
    MODE_BLINK_ALERT_DISMISS,  // Blink green - alert dismissed
    MODE_BLINK_OUTAGE_START,   // Blink blue - outage started
    MODE_BLINK_RESTORE,        // Blink yellow - power restored
    MODE_TEST                  // Test mode (manual control)
};

class RGBManager {
public:
    RGBManager();
    void begin();
    void update();

    // Event notifications (called by AlertManager and LightManager)
    void notifyAlertStarted();
    void notifyAlertDismissed();
    void notifyOutageStarted();
    void notifyPowerRestored();

    // Test mode control (for web interface)
    void setTestMode(bool enabled);
    void testBlinkAlertStart();
    void testBlinkAlertDismiss();
    void testBlinkOutageStart();
    void testBlinkRestore();
    void testAmbient();
    bool isInTestMode() { return testMode; }
    String getCurrentMode();

    // Get current state for web interface
    uint32_t getCurrentColor();
    bool isBlinking();

private:
    // State tracking
    bool previousAlertState;
    bool previousOutageState;
    RGBMode currentMode;
    unsigned long blinkStartTime;
    unsigned long lastBlinkToggle;
    bool blinkState;
    bool testMode;

    // Blink control
    void startBlink(RGBMode mode);
    void updateBlink();
    void updateAmbient();

    // RGB output
    void setRGBColor(uint32_t color, uint8_t brightness);
    void getRGBAmbientColor(uint32_t& color, uint8_t& brightness);
    uint32_t getBlinkColor(RGBMode mode);
};

extern RGBManager rgbManager;

#endif // RGBMANAGER_H

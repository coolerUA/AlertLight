#ifndef ALERTMANAGER_H
#define ALERTMANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../Config/Config.h"
#include "../RegionMapper/RegionMapper.h"

class AlertManager {
public:
    AlertManager();

    // Initialize
    void begin();

    // Call this in loop to handle periodic checks
    void update();

    // Force an immediate check (for testing)
    void forceCheck();

    // Force immediate update regardless of interval (for WiFi connection event)
    void forceUpdate();

    // Get debug info
    String getLastCallTime();
    int getLastHTTPCode();
    String getLastError();
    String getLastResponse();
    bool isAlertActive();
    String getRegionName();

private:
    unsigned long lastCheckTime;
    unsigned long lastSuccessTime;

    // Debug info
    String lastCallTimeStr;
    int lastHTTPCode;
    String lastError;
    String lastResponseData;

    // Alert state
    bool alertActive;
    bool previousAlertActive;  // Track previous state for RGB notifications
    String regionName;
    String alertStatus;

    // Perform the API check
    void checkAlert();

    // Parse JSON response
    bool parseResponse(const String& json);
};

extern AlertManager alertManager;

#endif // ALERTMANAGER_H

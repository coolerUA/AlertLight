#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include "../Config/Config.h"

struct OutageRange {
    String time_range;  // e.g., "08:30-12:30"
    bool is_active;     // true if current time is in this range
};

class LightManager {
public:
    LightManager();
    void begin();
    void update();
    void forceCheck();

    // Force immediate update regardless of interval (for WiFi connection event)
    void forceUpdate();

    // Recalculate active states based on current time (without API call)
    void updateActiveStates();

    // Get debug info
    String getLastCallTime();
    int getLastHTTPCode();
    String getLastError();
    String getLastResponse();
    bool isEmergencyShutdown();
    bool isCurrentlyOutage();
    String getQueue();

    // Get outage ranges (for dynamic UI display)
    const std::vector<OutageRange>& getOutageRanges() const;

private:
    unsigned long lastCheckTime;
    unsigned long lastSuccessTime;
    String lastCallTimeStr;
    int lastHTTPCode;
    String lastError;
    String lastResponseData;
    bool emergencyShutdown;
    bool currentOutage;
    bool previousOutageState;  // Track previous state for RGB notifications
    String queueName;

    // Outage ranges for display
    std::vector<OutageRange> outageRanges;

    void checkSchedule();
    bool parseResponse(const String& json);
    String formatTime(int minutes);
    void determineOutageStatus(JsonObject& todayData);
};

extern LightManager lightManager;
#endif

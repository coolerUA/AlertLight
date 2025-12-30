#include "LightManager.h"
#include "../AlertLight_UI/AlertLight_UI.h"
#include "../RGBManager/RGBManager.h"
#include <time.h>

LightManager lightManager;

LightManager::LightManager() {
    lastCheckTime = 0;
    lastSuccessTime = 0;
    lastHTTPCode = 0;
    emergencyShutdown = false;
    currentOutage = false;
    previousOutageState = false;
    lastCallTimeStr = "Never";
}

void LightManager::begin() {
    printf("Light Manager initialized\n");
}

void LightManager::update() {
    AlertLightConfig& cfg = configManager.getConfig();

    // Check if it's time to update
    if (millis() - lastCheckTime >= cfg.light_check_interval * 1000) {
        checkSchedule();
        lastCheckTime = millis();
    }
}

void LightManager::forceCheck() {
    checkSchedule();
    lastCheckTime = millis();
}

void LightManager::forceUpdate() {
    AlertLightConfig& cfg = configManager.getConfig();
    unsigned long checkInterval = cfg.light_check_interval * 1000;
    lastCheckTime = millis() - checkInterval - 1;
}

void LightManager::updateActiveStates() {
    // Only update if we have outage data
    if (outageRanges.size() == 0 && !emergencyShutdown) {
        return;
    }

    // Get current time
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    int currentMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;

    // Recalculate active state for each outage range
    bool hasActiveOutage = false;
    bool stateChanged = false;

    for (size_t i = 0; i < outageRanges.size(); i++) {
        // Parse the time range to get start and end minutes
        String timeRange = outageRanges[i].time_range;
        int dashPos = timeRange.indexOf('-');
        if (dashPos > 0) {
            String startStr = timeRange.substring(0, dashPos);
            String endStr = timeRange.substring(dashPos + 1);

            int startHour = startStr.substring(0, 2).toInt();
            int startMin = startStr.substring(3, 5).toInt();
            int endHour = endStr.substring(0, 2).toInt();
            int endMin = endStr.substring(3, 5).toInt();

            int startMinutes = startHour * 60 + startMin;
            int endMinutes = endHour * 60 + endMin;

            bool wasActive = outageRanges[i].is_active;
            outageRanges[i].is_active = (currentMinutes >= startMinutes && currentMinutes < endMinutes);

            if (wasActive != outageRanges[i].is_active) {
                stateChanged = true;
            }

            if (outageRanges[i].is_active) {
                hasActiveOutage = true;
            }
        }
    }

    // Update currentOutage flag if changed
    if (currentOutage != hasActiveOutage) {
        currentOutage = hasActiveOutage;
        stateChanged = true;
    }

    // Update UI if state changed
    if (stateChanged) {
        AlertLightConfig& cfg = configManager.getConfig();

        if (emergencyShutdown) {
            // Emergency shutdown - no change needed
            return;
        } else if (outageRanges.size() == 0) {
            // No outages - no change needed
            return;
        } else {
            // Convert vector to array for UI
            outage_time_slot_t slots[outageRanges.size()];
            printf("\n=== Light UI Update (updateActiveStates) ===\n");
            printf("Queue: %s, Slots: %d\n", queueName.c_str(), outageRanges.size());
            for (size_t i = 0; i < outageRanges.size(); i++) {
                slots[i].time_range = outageRanges[i].time_range.c_str();
                slots[i].is_active = outageRanges[i].is_active;
                printf("  Slot %d: %s (active=%d)\n", i, slots[i].time_range, slots[i].is_active);
            }
            AlertLight_UI_Update_Light(queueName.c_str(), slots, outageRanges.size());
            printf("=================================\n\n");

            if (currentOutage) {
                AlertLight_UI_Update_LightIndicator_Emergency(true);
            } else {
                AlertLight_UI_Update_LightIndicator(true);
            }
        }
    }
}

void LightManager::checkSchedule() {
    AlertLightConfig& cfg = configManager.getConfig();

    // Get current time for logging
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    lastCallTimeStr = String(timeStr);

    HTTPClient http;
    http.begin(cfg.light_api_url);
    http.setTimeout(10000);

    lastHTTPCode = http.GET();

    if (lastHTTPCode == 200) {
        lastResponseData = http.getString();

        if (parseResponse(lastResponseData)) {
            lastError = "";
            lastSuccessTime = millis();

            // Update UI with dynamic outage slots
            if (emergencyShutdown) {
                // Emergency shutdown - create single slot
                outage_time_slot_t emergency_slot;
                emergency_slot.time_range = "Екстрені відключення";
                emergency_slot.is_active = true;
                AlertLight_UI_Update_Light(queueName.c_str(), &emergency_slot, 1);
                AlertLight_UI_Update_LightIndicator_Emergency(true);
            } else if (outageRanges.size() == 0) {
                // No outages
                outage_time_slot_t no_outage_slot;
                no_outage_slot.time_range = "Немає";
                no_outage_slot.is_active = false;
                AlertLight_UI_Update_Light(queueName.c_str(), &no_outage_slot, 1);
                AlertLight_UI_Update_LightIndicator(false);
            } else {
                // Convert vector to array for UI
                outage_time_slot_t slots[outageRanges.size()];
                printf("\n=== Light UI Update (checkSchedule) ===\n");
                printf("Queue: %s, Slots: %d\n", queueName.c_str(), outageRanges.size());
                for (size_t i = 0; i < outageRanges.size(); i++) {
                    slots[i].time_range = outageRanges[i].time_range.c_str();
                    slots[i].is_active = outageRanges[i].is_active;
                    printf("  Slot %d: %s (active=%d)\n", i, slots[i].time_range, slots[i].is_active);
                }
                AlertLight_UI_Update_Light(queueName.c_str(), slots, outageRanges.size());
                printf("=================================\n\n");

                if (currentOutage) {
                    AlertLight_UI_Update_LightIndicator_Emergency(true);
                } else {
                    AlertLight_UI_Update_LightIndicator(true);
                }
            }
        } else {
            lastError = "Failed to parse JSON response";
            printf("Light parse error\n");
        }
    } else {
        lastError = "HTTP request failed: " + String(lastHTTPCode);
        printf("Light API error: %d\n", lastHTTPCode);
        lastResponseData = "";
    }

    http.end();
}

bool LightManager::parseResponse(const String& json) {
    StaticJsonDocument<8192> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        return false;
    }

    AlertLightConfig& cfg = configManager.getConfig();
    queueName = String(cfg.light_queue);

    if (!doc.containsKey(cfg.light_queue)) {
        return false;
    }

    JsonObject queueData = doc[cfg.light_queue];
    if (!queueData.containsKey("today")) {
        return false;
    }

    JsonObject todayData = queueData["today"];
    String status = todayData["status"].as<String>();

    if (status == "EmergencyShutdowns") {
        emergencyShutdown = true;
        currentOutage = true;
        return true;
    }

    emergencyShutdown = false;
    determineOutageStatus(todayData);

    return true;
}

void LightManager::determineOutageStatus(JsonObject& todayData) {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    int currentMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;

    bool wasInOutage = currentOutage;
    currentOutage = false;
    outageRanges.clear();

    JsonArray slots = todayData["slots"].as<JsonArray>();

    for (JsonVariant v : slots) {
        JsonObject slot = v.as<JsonObject>();
        String type = slot["type"].as<String>();

        if (type == "Definite") {
            int start = slot["start"].as<int>();
            int end = slot["end"].as<int>();

            OutageRange range;
            range.time_range = formatTime(start) + "-" + formatTime(end);
            range.is_active = (currentMinutes >= start && currentMinutes < end);

            if (range.is_active) {
                currentOutage = true;
            }

            outageRanges.push_back(range);
        }
    }

    // Detect state changes and notify RGB manager
    if (currentOutage != wasInOutage) {
        if (currentOutage) {
            // Outage started
            rgbManager.notifyOutageStarted();
        } else if (!wasInOutage && previousOutageState) {
            // Power restored (was in outage, now not)
            rgbManager.notifyPowerRestored();
        }
        previousOutageState = currentOutage;
    }
}

String LightManager::formatTime(int minutes) {
    int hours = minutes / 60;
    int mins = minutes % 60;
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", hours, mins);
    return String(buf);
}

// Getters for debug info
String LightManager::getLastCallTime() {
    return lastCallTimeStr;
}

int LightManager::getLastHTTPCode() {
    return lastHTTPCode;
}

String LightManager::getLastError() {
    return lastError;
}

String LightManager::getLastResponse() {
    return lastResponseData;
}

bool LightManager::isEmergencyShutdown() {
    return emergencyShutdown;
}

bool LightManager::isCurrentlyOutage() {
    return currentOutage;
}

String LightManager::getQueue() {
    return queueName;
}

const std::vector<OutageRange>& LightManager::getOutageRanges() const {
    return outageRanges;
}

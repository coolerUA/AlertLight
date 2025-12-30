#include "AlertManager.h"
#include "../AlertLight_UI/AlertLight_UI.h"
#include "../RGBManager/RGBManager.h"
#include <WiFi.h>
#include <time.h>

AlertManager alertManager;

AlertManager::AlertManager() {
    lastCheckTime = 0;
    lastSuccessTime = 0;
    lastHTTPCode = 0;
    alertActive = false;
    previousAlertActive = false;
    regionName = "Unknown";
    alertStatus = "Not checked yet";
    lastCallTimeStr = "Never";
    lastError = "";
    lastResponseData = "";
}

void AlertManager::begin() {
    printf("AlertManager initialized\n");
}

void AlertManager::update() {
    AlertLightConfig& cfg = configManager.getConfig();

    // Check if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    // Check if it's time for a periodic check
    unsigned long now = millis();
    if (now - lastCheckTime >= cfg.alert_check_interval * 1000) {
        checkAlert();
    }
}

void AlertManager::forceCheck() {
    checkAlert();
}

void AlertManager::forceUpdate() {
    AlertLightConfig& cfg = configManager.getConfig();
    unsigned long checkInterval = cfg.alert_check_interval * 1000;
    lastCheckTime = millis() - checkInterval - 1;
}

void AlertManager::checkAlert() {
    AlertLightConfig& cfg = configManager.getConfig();
    lastCheckTime = millis();

    printf("\n=== Checking Alert API for region %d ===\n", cfg.alert_region_id);
    printf("Current state - alertActive: %d, previousAlertActive: %d\n", alertActive, previousAlertActive);

    unsigned long uptime = millis() / 1000;
    unsigned long hours = uptime / 3600;
    unsigned long minutes = (uptime % 3600) / 60;
    unsigned long seconds = uptime % 60;
    char timeStr[32];
    snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu:%02lu", hours, minutes, seconds);
    lastCallTimeStr = String(timeStr);

    String url = "https://air-save.ops.ajax.systems/api/mobile/status/regions/v2?regions=" + String(cfg.alert_region_id);

    HTTPClient http;
    http.begin(url);
    http.setTimeout(10000);

    int httpCode = http.GET();
    lastHTTPCode = httpCode;
    printf("HTTP Response: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        lastResponseData = payload;

        if (parseResponse(payload)) {
            lastError = "";
            lastSuccessTime = millis();
            AlertLight_UI_Update_Alert(regionName.c_str(), alertStatus.c_str(), alertActive);
        } else {
            lastError = "Failed to parse JSON response";
            printf("Alert parse error\n");
        }
    } else if (httpCode > 0) {
        lastError = "HTTP error " + String(httpCode) + ": " + http.errorToString(httpCode);
        lastResponseData = "";
        printf("Alert API error: %d\n", httpCode);
    } else {
        lastError = "Connection failed: " + http.errorToString(httpCode);
        lastResponseData = "";
        printf("Alert connection error\n");
    }

    http.end();
}

bool AlertManager::parseResponse(const String& json) {
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        printf("JSON parse error\n");
        return false;
    }

    if (!doc.containsKey("alarms") || !doc["alarms"].is<JsonArray>()) {
        printf("No 'alarms' array in response\n");
        return false;
    }

    JsonArray alarms = doc["alarms"].as<JsonArray>();
    AlertLightConfig& cfg = configManager.getConfig();

    regionName = RegionMapper::getRegionName(cfg.alert_region_id);

    // Determine alert status based on response
    if (alarms.size() == 0) {
        printf("No alarms in response - setting alertActive=false\n");
        alertActive = false;
        alertStatus = "No Alert";
    } else {
        printf("Searching %d alarms for region %d\n", alarms.size(), cfg.alert_region_id);
        bool foundOurRegion = false;
        for (JsonVariant v : alarms) {
            JsonObject alarm = v.as<JsonObject>();
            int alarmRegionId = alarm["regionId"].as<int>();

            if (alarm.containsKey("regionId") && alarmRegionId == cfg.alert_region_id) {
                foundOurRegion = true;
                alertActive = true;
                printf("FOUND region %d in alarms - setting alertActive=true\n", cfg.alert_region_id);

                if (alarm.containsKey("alarmType")) {
                    alertStatus = alarm["alarmType"].as<String>() + " Alert";
                } else {
                    alertStatus = "Active Alert";
                }
                break;
            }
        }

        if (!foundOurRegion) {
            printf("Region %d NOT found in alarms - setting alertActive=false\n", cfg.alert_region_id);
            alertActive = false;
            alertStatus = "No Alert";
        }
    }

    // Detect state changes and notify RGB manager
    if (alertActive != previousAlertActive) {
        if (alertActive) {
            printf("Alert state changed: NO ALERT -> ALERT (triggering red blink)\n");
            rgbManager.notifyAlertStarted();
        } else {
            printf("Alert state changed: ALERT -> NO ALERT (triggering green blink)\n");
            rgbManager.notifyAlertDismissed();
        }
        previousAlertActive = alertActive;
    }

    return true;
}

String AlertManager::getLastCallTime() {
    return lastCallTimeStr;
}

int AlertManager::getLastHTTPCode() {
    return lastHTTPCode;
}

String AlertManager::getLastError() {
    return lastError;
}

String AlertManager::getLastResponse() {
    return lastResponseData;
}

bool AlertManager::isAlertActive() {
    return alertActive;
}

String AlertManager::getRegionName() {
    return regionName;
}

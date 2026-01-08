#ifndef WEBCONFIG_H
#define WEBCONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "../Config/Config.h"
#include "../AlertManager/AlertManager.h"
#include "../RegionMapper/RegionMapper.h"

class WebConfigManager {
public:
    WebConfigManager();

    // Initialize WiFi and web server
    void begin();

    // Handle web server requests (call in loop)
    void handleClient();

    // Get current connection status
    bool isConnected();
    String getIPAddress();
    String getSSID();

    // Start AP mode
    void startAPMode();

    // Try to connect to WiFi
    bool connectToWiFi(unsigned long timeout_ms = 10000);

private:
    WebServer server;
    DNSServer dnsServer;
    bool apMode;
    bool wifiConnected;
    String apSSID;
    unsigned long lastWiFiCheck;
    unsigned long wifiCheckInterval;
    unsigned long lastScanAttempt;
    unsigned long scanRetryInterval;
    String statusLog;

    // Web page handlers
    void handleRoot();
    void handleWiFiConfig();
    void handleAlertConfig();
    void handleLightConfig();
    void handleRGBConfig();
    void handleStatus();
    void handleSaveWiFi();
    void handleSaveAlert();
    void handleSaveLight();
    void handleSaveRGB();
    void handleRestart();
    void handleNotFound();
    void handleScan();
    void handleTestAlert();
    void handleTestLight();
    void handleNTP();
    void handleSyncNTP();
    void handleTestRGB();

    // Helper functions
    void addLog(const String& message);
    void checkWiFiStatus();
    void syncNTPTime();  // Synchronize time with NTP server

    // HTML generation helpers
    String generateHeader(const char* title);
    String generateFooter();
    String generateNavigation();
    String colorToHex(uint32_t color);
    uint32_t hexToColor(String hex);
};

extern WebConfigManager webConfig;

#endif // WEBCONFIG_H

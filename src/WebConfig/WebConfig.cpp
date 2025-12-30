#include "WebConfig.h"
#include "../AlertLight_UI/AlertLight_UI.h"
#include "../LightManager/LightManager.h"
#include "../RGBManager/RGBManager.h"
#include <time.h>

WebConfigManager webConfig;

WebConfigManager::WebConfigManager() : server(8080), apMode(false), wifiConnected(false),
    lastWiFiCheck(0), wifiCheckInterval(5000), lastScanAttempt(0), scanRetryInterval(60000) {
    statusLog = "";
}

void WebConfigManager::begin() {
    // Get MAC address for unique AP SSID
    uint8_t mac[6];
    WiFi.macAddress(mac);
    apSSID = "AlertLight-" + String(mac[4], HEX) + String(mac[5], HEX);
    apSSID.toUpperCase();

    // Try to connect to saved WiFi
    AlertLightConfig& cfg = configManager.getConfig();

    addLog("System started");

    if (strlen(cfg.wifi_ssid) > 0) {
        addLog("Trying to connect to WiFi: " + String(cfg.wifi_ssid));
        if (connectToWiFi(15000)) {
            wifiConnected = true;
            apMode = false;
            addLog("Connected! IP: " + WiFi.localIP().toString());
            syncNTPTime();  // Synchronize time with NTP server
        } else {
            addLog("Failed to connect, starting AP mode");
            startAPMode();
        }
    } else {
        addLog("No WiFi credentials, starting AP mode");
        startAPMode();
    }

    // Setup web server routes
    server.on("/", [this]() { this->handleRoot(); });
    server.on("/wifi", [this]() { this->handleWiFiConfig(); });
    server.on("/alert", [this]() { this->handleAlertConfig(); });
    server.on("/light", [this]() { this->handleLightConfig(); });
    server.on("/rgb", [this]() { this->handleRGBConfig(); });
    server.on("/status", [this]() { this->handleStatus(); });
    server.on("/save_wifi", [this]() { this->handleSaveWiFi(); });
    server.on("/save_alert", [this]() { this->handleSaveAlert(); });
    server.on("/save_light", [this]() { this->handleSaveLight(); });
    server.on("/save_rgb", [this]() { this->handleSaveRGB(); });
    server.on("/restart", [this]() { this->handleRestart(); });
    server.on("/scan", [this]() { this->handleScan(); });
    server.on("/test_alert", [this]() { this->handleTestAlert(); });
    server.on("/api/test_light", [this]() { this->handleTestLight(); });
    server.on("/ntp", [this]() { this->handleNTP(); });
    server.on("/sync_ntp", [this]() { this->handleSyncNTP(); });
    server.on("/test_rgb", [this]() { this->handleTestRGB(); });
    server.onNotFound([this]() { this->handleNotFound(); });

    server.begin();
    printf("Web server started on port %d\n", cfg.web_port);
}

void WebConfigManager::handleClient() {
    if (apMode) {
        dnsServer.processNextRequest();
    }
    server.handleClient();

    // Check WiFi status periodically
    unsigned long now = millis();
    if (now - lastWiFiCheck >= wifiCheckInterval) {
        lastWiFiCheck = now;
        checkWiFiStatus();
    }
}

bool WebConfigManager::isConnected() {
    return wifiConnected && WiFi.status() == WL_CONNECTED;
}

String WebConfigManager::getIPAddress() {
    if (apMode) {
        return WiFi.softAPIP().toString();
    } else {
        return WiFi.localIP().toString();
    }
}

String WebConfigManager::getSSID() {
    if (apMode) {
        return apSSID;
    } else {
        return WiFi.SSID();
    }
}

void WebConfigManager::startAPMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID.c_str());

    // Start DNS server for captive portal
    dnsServer.start(53, "*", WiFi.softAPIP());

    apMode = true;
    wifiConnected = false;

    printf("AP Mode started\n");
    printf("SSID: %s\n", apSSID.c_str());
    printf("IP: %s\n", WiFi.softAPIP().toString().c_str());
}

bool WebConfigManager::connectToWiFi(unsigned long timeout_ms) {
    AlertLightConfig& cfg = configManager.getConfig();

    // Disconnect and clean up first
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    // Configure static IP if enabled, otherwise use DHCP
    if (cfg.use_static_ip) {
        IPAddress ip, gateway, subnet;
        ip.fromString(cfg.static_ip);
        gateway.fromString(cfg.static_gateway);
        subnet.fromString(cfg.static_subnet);
        WiFi.config(ip, gateway, subnet);
        addLog("Using static IP: " + String(cfg.static_ip));
    } else {
        // Reset to DHCP
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
        addLog("Using DHCP");
    }

    // Scan for networks to verify SSID is available
    addLog("Scanning for networks...");
    delay(300);  // Give time to see the message

    // Extended scan time for environments with many networks (52+)
    // 2000ms per channel ensures thorough discovery of all networks
    int networks = WiFi.scanNetworks(false, false, false, 2000); // async=false, show_hidden=false, passive=false, max_ms_per_chan=2000

    lastScanAttempt = millis();

    if (networks < 0) {
        addLog("Scan failed!");
        delay(500);
        return false;
    }

    char buf[64];
    snprintf(buf, sizeof(buf), "Found %d networks", networks);
    addLog(buf);
    delay(1500);  // Give time to see the result

    bool ssidFound = false;
    int8_t targetRSSI = 0;
    const char* targetEncryption = "";

    // Check up to 30 networks (balance between finding networks and memory usage)
    int maxCheck = min(networks, 50);

    addLog("Checking networks...");
    delay(300);

    for (int i = 0; i < maxCheck; i++) {
        String ssid = WiFi.SSID(i);
        int8_t rssi = WiFi.RSSI(i);

        // Log first 5 networks for debugging
        if (i < 5 && ssid.length() > 0) {
            snprintf(buf, sizeof(buf), "  %s (%d dBm)", ssid.c_str(), rssi);
            addLog(buf);
            delay(200);  // Delay between each network for visibility
        }

        // Check if this is our target SSID
        if (ssid == String(cfg.wifi_ssid)) {
            ssidFound = true;
            targetRSSI = rssi;

            switch (WiFi.encryptionType(i)) {
                case WIFI_AUTH_OPEN: targetEncryption = "OPEN"; break;
                case WIFI_AUTH_WEP: targetEncryption = "WEP"; break;
                case WIFI_AUTH_WPA_PSK: targetEncryption = "WPA"; break;
                case WIFI_AUTH_WPA2_PSK: targetEncryption = "WPA2"; break;
                case WIFI_AUTH_WPA_WPA2_PSK: targetEncryption = "WPA/WPA2"; break;
                case WIFI_AUTH_WPA2_ENTERPRISE: targetEncryption = "WPA2-Enterprise"; break;
                default: targetEncryption = "Unknown"; break;
            }

            snprintf(buf, sizeof(buf), ">>> Found: %s (%s, %d dBm)", cfg.wifi_ssid, targetEncryption, targetRSSI);
            addLog(buf);
            delay(500);
            break; // Found it, stop searching
        }
    }

    WiFi.scanDelete();

    if (!ssidFound) {
        snprintf(buf, sizeof(buf), "SSID '%s' not found in first %d networks", cfg.wifi_ssid, maxCheck);
        addLog(buf);
        return false;
    }

    addLog("Connecting to: " + String(cfg.wifi_ssid));
    WiFi.begin(cfg.wifi_ssid, cfg.wifi_password);

    unsigned long start = millis();
    wl_status_t status;

    // Wait for connection
    while ((status = WiFi.status()) != WL_CONNECTED && millis() - start < timeout_ms) {
        delay(500);
        printf(".");
    }
    printf("\n");

    if (status == WL_CONNECTED) {
        // Wait for IP address (important for DHCP)
        unsigned long ipWaitStart = millis();
        while (WiFi.localIP().toString() == "0.0.0.0" && millis() - ipWaitStart < 5000) {
            delay(100);
            printf("*");
        }
        printf("\n");

        IPAddress ip = WiFi.localIP();
        if (ip.toString() != "0.0.0.0") {
            addLog("WiFi connected - IP: " + ip.toString());
            addLog("Gateway: " + WiFi.gatewayIP().toString());
            addLog("DNS: " + WiFi.dnsIP().toString());
            addLog("RSSI: " + String(WiFi.RSSI()) + " dBm");
            return true;
        } else {
            addLog("WiFi connected but no IP address (DHCP failed)");
            WiFi.disconnect(true);
            return false;
        }
    } else {
        String statusMsg = "Status: " + String(status);
        switch (status) {
            case WL_NO_SSID_AVAIL: statusMsg += " (SSID not found)"; break;
            case WL_CONNECT_FAILED: statusMsg += " (Connection failed)"; break;
            case WL_CONNECTION_LOST: statusMsg += " (Connection lost)"; break;
            case WL_DISCONNECTED: statusMsg += " (Disconnected)"; break;
            default: break;
        }
        addLog("WiFi connection failed - " + statusMsg);
        return false;
    }
}

// HTML Helper Functions
String WebConfigManager::generateHeader(const char* title) {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>" + String(title) + " - AlertLight</title>";
    html += "<style>";
    html += "body { font-family: Arial; margin: 0; padding: 20px; background: #1a1a1a; color: #fff; }";
    html += "h1 { color: #00aaff; }";
    html += "h2 { color: #00ff00; margin-top: 30px; }";
    html += ".container { max-width: 600px; margin: 0 auto; }";
    html += ".nav { background: #2a2a2a; padding: 10px; margin-bottom: 20px; border-radius: 5px; }";
    html += ".nav a { color: #00aaff; text-decoration: none; margin-right: 15px; }";
    html += ".nav a:hover { color: #00ff00; }";
    html += ".form-group { margin-bottom: 15px; }";
    html += "label { display: block; margin-bottom: 5px; color: #aaa; }";
    html += "input, select { width: 100%; padding: 8px; box-sizing: border-box; background: #2a2a2a; ";
    html += "border: 1px solid #444; color: #fff; border-radius: 3px; }";
    html += "input[type='checkbox'] { width: auto; }";
    html += "input[type='color'] { height: 40px; }";
    html += "button { background: #00aaff; color: #000; padding: 10px 20px; border: none; ";
    html += "border-radius: 3px; cursor: pointer; font-weight: bold; margin-top: 10px; }";
    html += "button:hover { background: #00ff00; }";
    html += ".status { padding: 10px; background: #2a2a2a; border-radius: 3px; margin-bottom: 20px; }";
    html += ".success { color: #00ff00; }";
    html += ".error { color: #ff0000; }";
    html += ".network-item { padding: 12px; border-bottom: 1px solid #444; cursor: pointer; transition: background 0.2s; }";
    html += ".network-item:hover { background: #3a3a3a; }";
    html += ".network-name { font-size: 16px; font-weight: bold; color: #fff; margin-bottom: 5px; }";
    html += ".network-details { font-size: 12px; color: #aaa; }";
    html += ".signal-strong { color: #00ff00; }";
    html += ".signal-good { color: #80ff00; }";
    html += ".signal-fair { color: #ffaa00; }";
    html += ".signal-weak { color: #ff6600; }";
    html += "</style></head><body><div class='container'>";
    html += "<h1>AlertLight Configuration</h1>";
    html += generateNavigation();
    return html;
}

String WebConfigManager::generateFooter() {
    return "</div></body></html>";
}

String WebConfigManager::generateNavigation() {
    String nav = "<div class='nav'>";
    nav += "<a href='/'>Home</a>";
    nav += "<a href='/wifi'>WiFi</a>";
    nav += "<a href='/alert'>Alert API</a>";
    nav += "<a href='/light'>Light API</a>";
    nav += "<a href='/rgb'>RGB LED</a>";
    nav += "<a href='/ntp'>Time/NTP</a>";
    nav += "<a href='/status'>Status</a>";
    nav += "<a href='/restart'>Restart</a>";
    nav += "</div>";
    return nav;
}

String WebConfigManager::colorToHex(uint32_t color) {
    char hex[8];
    sprintf(hex, "#%06X", color);
    return String(hex);
}

uint32_t WebConfigManager::hexToColor(String hex) {
    if (hex.startsWith("#")) {
        hex = hex.substring(1);
    }
    return strtoul(hex.c_str(), NULL, 16);
}

// Web Handlers
void WebConfigManager::handleRoot() {
    AlertLightConfig& cfg = configManager.getConfig();

    String html = generateHeader("Home");
    html += "<h2>Status</h2>";
    html += "<div class='status'>";
    html += "<p><strong>WiFi Mode:</strong> " + String(apMode ? "Access Point" : "Station") + "</p>";
    html += "<p><strong>SSID:</strong> " + getSSID() + "</p>";
    html += "<p><strong>IP Address:</strong> " + getIPAddress() + "</p>";
    html += "<p><strong>Connection:</strong> " + String(isConnected() ? "Connected" : "Not Connected") + "</p>";
    html += "</div>";

    html += "<h2>Quick Settings</h2>";
    html += "<p><a href='/wifi'>Configure WiFi</a></p>";
    html += "<p><a href='/alert'>Configure Alert Monitoring</a></p>";
    html += "<p><a href='/light'>Configure Light Outages</a></p>";
    html += "<p><a href='/rgb'>Configure RGB LED</a></p>";

    html += generateFooter();
    server.send(200, "text/html", html);
}

void WebConfigManager::handleWiFiConfig() {
    AlertLightConfig& cfg = configManager.getConfig();

    String html = generateHeader("WiFi Configuration");

    // Add network scanner section
    html += "<h2>Available Networks</h2>";
    html += "<div class='form-group'>";
    html += "<button type='button' onclick='scanNetworks()' id='scanBtn'>Scan for Networks</button>";
    html += "<div id='scanStatus' style='margin-top: 10px; color: #00aaff;'></div>";
    html += "</div>";
    html += "<div id='networkList' style='max-height: 300px; overflow-y: auto; margin-bottom: 20px;'></div>";

    html += "<h2>WiFi Settings</h2>";
    html += "<form action='/save_wifi' method='POST'>";

    html += "<div class='form-group'>";
    html += "<label>SSID:</label>";
    html += "<input type='text' id='ssid' name='ssid' value='" + String(cfg.wifi_ssid) + "' required>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Password:</label>";
    html += "<input type='password' name='password' value='" + String(cfg.wifi_password) + "'>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label><input type='checkbox' name='use_static' " + String(cfg.use_static_ip ? "checked" : "") + "> Use Static IP</label>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Static IP:</label>";
    html += "<input type='text' name='static_ip' value='" + String(cfg.static_ip) + "'>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Gateway:</label>";
    html += "<input type='text' name='gateway' value='" + String(cfg.static_gateway) + "'>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Subnet:</label>";
    html += "<input type='text' name='subnet' value='" + String(cfg.static_subnet) + "'>";
    html += "</div>";

    html += "<button type='submit'>Save WiFi Settings</button>";
    html += "</form>";

    // Add JavaScript for network scanning
    html += "<script>";
    html += "function getSignalInfo(rssi) {";
    html += "  if (rssi >= -50) return { bars: '████', percent: 100, class: 'signal-strong' };";
    html += "  if (rssi >= -60) return { bars: '███▯', percent: 75, class: 'signal-good' };";
    html += "  if (rssi >= -70) return { bars: '██▯▯', percent: 50, class: 'signal-fair' };";
    html += "  if (rssi >= -80) return { bars: '█▯▯▯', percent: 25, class: 'signal-weak' };";
    html += "  return { bars: '▯▯▯▯', percent: 10, class: 'signal-weak' };";
    html += "}";
    html += "function getLockIcon(enc) {";
    html += "  return enc === 'Open' ? '[Open]' : '[Secure]';";
    html += "}";
    html += "function selectNetwork(ssid, bssid) {";
    html += "  document.getElementById('ssid').value = ssid;";
    html += "  document.getElementById('ssid').focus();";
    html += "}";
    html += "function scanNetworks() {";
    html += "  const btn = document.getElementById('scanBtn');";
    html += "  const status = document.getElementById('scanStatus');";
    html += "  const list = document.getElementById('networkList');";
    html += "  btn.disabled = true;";
    html += "  status.textContent = 'Scanning for networks (this may take 20-30 seconds)...';";
    html += "  list.innerHTML = '';";
    html += "  fetch('/scan').then(r => r.json()).then(data => {";
    html += "    status.textContent = 'Found ' + data.count + ' networks';";
    html += "    if (data.networks && data.networks.length > 0) {";
    html += "      let html = '<div style=\"background:#2a2a2a; border-radius:5px;\">';";
    html += "      data.networks.forEach(net => {";
    html += "        const sig = getSignalInfo(net.rssi);";
    html += "        html += '<div class=\"network-item\" ';";
    html += "        html += 'onclick=\"selectNetwork(\\'' + net.ssid.replace(/'/g, \"\\\\'\") + '\\', \\'' + net.bssid + '\\')\">';";
    html += "        html += '<div class=\"network-name\">' + net.ssid + '</div>';";
    html += "        html += '<div class=\"network-details\">';";
    html += "        html += '<span class=\"' + sig.class + '\">' + sig.bars + ' ' + net.rssi + ' dBm (' + sig.percent + '%)</span>';";
    html += "        html += ' | ' + getLockIcon(net.encryption) + ' ' + net.encryption;";
    html += "        html += ' | ' + net.bssid;";
    html += "        html += '</div></div>';";
    html += "      });";
    html += "      html += '</div>';";
    html += "      list.innerHTML = html;";
    html += "    }";
    html += "    btn.disabled = false;";
    html += "  }).catch(err => {";
    html += "    status.textContent = 'Scan failed: ' + err;";
    html += "    btn.disabled = false;";
    html += "  });";
    html += "}";
    html += "window.onload = function() { scanNetworks(); };";
    html += "</script>";

    html += generateFooter();
    server.send(200, "text/html", html);
}

void WebConfigManager::handleAlertConfig() {
    AlertLightConfig& cfg = configManager.getConfig();

    String html = generateHeader("Alert Configuration");

    // Debug info section
    html += "<h2>API Debug Info</h2>";
    html += "<div class='status'>";
    html += "<p><strong>Last Check:</strong> " + alertManager.getLastCallTime() + "</p>";
    html += "<p><strong>HTTP Status:</strong> ";
    int httpCode = alertManager.getLastHTTPCode();
    if (httpCode == 200) {
        html += "<span class='success'>" + String(httpCode) + " OK</span>";
    } else if (httpCode > 0) {
        html += "<span class='error'>" + String(httpCode) + "</span>";
    } else {
        html += "Not checked yet";
    }
    html += "</p>";
    html += "<p><strong>Region:</strong> " + alertManager.getRegionName() + "</p>";
    html += "<p><strong>Alert Status:</strong> ";
    if (alertManager.isAlertActive()) {
        html += "<span class='error'>ACTIVE ALERT</span>";
    } else {
        html += "<span class='success'>No Alert</span>";
    }
    html += "</p>";
    if (alertManager.getLastError().length() > 0) {
        html += "<p><strong>Last Error:</strong> <span class='error'>" + alertManager.getLastError() + "</span></p>";
    }
    html += "<p><strong>Response Data:</strong></p>";
    html += "<div style='font-family: monospace; font-size: 11px; background: #000; color: #0f0; padding: 10px; ";
    html += "max-height: 200px; overflow-y: auto; word-break: break-all;'>";
    if (alertManager.getLastResponse().length() > 0) {
        html += alertManager.getLastResponse();
    } else {
        html += "No data";
    }
    html += "</div>";
    html += "<button type='button' onclick='testNow()' style='margin-top: 10px;'>Test API Now</button>";
    html += "</div>";

    html += "<h2>Alert Monitoring Settings</h2>";
    html += "<form action='/save_alert' method='POST'>";

    html += "<div class='form-group'>";
    html += "<label>Region:</label>";
    html += "<select name='region_id' required>";

    // Generate region dropdown options
    for (int i = 0; i < RegionMapper::getRegionCount(); i++) {
        const RegionInfo& region = RegionMapper::getRegion(i);
        html += "<option value='" + String(region.id) + "'";
        if (region.id == cfg.alert_region_id) {
            html += " selected";
        }
        html += ">" + String(region.name) + "</option>";
    }

    html += "</select>";
    html += "<small style='color: #aaa;'>API URL: https://air-save.ops.ajax.systems/api/mobile/status/regions/v2?regions={regionId}</small>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Check Interval (seconds):</label>";
    html += "<input type='number' name='interval' value='" + String(cfg.alert_check_interval) + "' required>";
    html += "</div>";

    html += "<button type='submit'>Save Alert Settings</button>";
    html += "</form>";

    // JavaScript for test button
    html += "<script>";
    html += "function testNow() {";
    html += "  fetch('/test_alert').then(r => r.text()).then(data => {";
    html += "    alert('API test triggered. Check debug info above (page will refresh in 3 seconds)');";
    html += "    setTimeout(() => location.reload(), 3000);";
    html += "  }).catch(err => alert('Test failed: ' + err));";
    html += "}";
    html += "</script>";

    html += generateFooter();
    server.send(200, "text/html", html);
}

void WebConfigManager::handleLightConfig() {
    AlertLightConfig& cfg = configManager.getConfig();

    String html = generateHeader("Light Outage Configuration");

    // Debug info section
    html += "<h2>API Debug Info</h2>";
    html += "<div class='status'>";
    html += "<p><strong>Last Check:</strong> " + lightManager.getLastCallTime() + "</p>";
    html += "<p><strong>HTTP Status:</strong> ";
    int httpCode = lightManager.getLastHTTPCode();
    if (httpCode == 200) {
        html += "<span class='success'>" + String(httpCode) + " OK</span>";
    } else if (httpCode > 0) {
        html += "<span class='error'>" + String(httpCode) + "</span>";
    } else {
        html += "Not checked yet";
    }
    html += "</p>";
    html += "<p><strong>Queue:</strong> " + lightManager.getQueue() + "</p>";

    if (lightManager.isEmergencyShutdown()) {
        html += "<p><strong>Status:</strong> <span class='error'>EMERGENCY SHUTDOWNS</span></p>";
    } else {
        html += "<p><strong>Currently in Outage:</strong> ";
        if (lightManager.isCurrentlyOutage()) {
            html += "<span class='error'>YES</span>";
        } else {
            html += "<span class='success'>NO</span>";
        }
        html += "</p>";

        // Display outage ranges dynamically
        const std::vector<OutageRange>& ranges = lightManager.getOutageRanges();
        if (ranges.size() > 0) {
            html += "<p><strong>Outages Today:</strong></p><ul style='margin: 5px 0;'>";
            for (size_t i = 0; i < ranges.size(); i++) {
                html += "<li>" + ranges[i].time_range;
                if (ranges[i].is_active) {
                    html += " <strong style='color: #ffff00;'>(ACTIVE NOW)</strong>";
                }
                html += "</li>";
            }
            html += "</ul>";
        } else {
            html += "<p><strong>Outages Today:</strong> None</p>";
        }
    }

    String lastError = lightManager.getLastError();
    if (lastError.length() > 0) {
        html += "<p><strong>Last Error:</strong> <span class='error'>" + lastError + "</span></p>";
    }

    String response = lightManager.getLastResponse();
    if (response.length() > 0) {
        html += "<p><strong>Response Data:</strong></p>";
        html += "<pre style='max-height: 200px; overflow-y: auto; background: #222; padding: 10px; border-radius: 5px;'>";
        if (response.length() > 1000) {
            html += response.substring(0, 1000) + "... (truncated)";
        } else {
            html += response;
        }
        html += "</pre>";
    }

    html += "<button type='button' onclick='testNow()'>Test API Now</button>";
    html += "</div>";

    html += "<script>";
    html += "function testNow() {";
    html += "  fetch('/api/test_light').then(() => location.reload());";
    html += "}";
    html += "</script>";

    html += "<h2>Light Outage Settings</h2>";
    html += "<form action='/save_light' method='POST'>";

    html += "<div class='form-group'>";
    html += "<label>Light Outage API URL:</label>";
    html += "<input type='text' name='api_url' value='" + String(cfg.light_api_url) + "' required>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Light Queue:</label>";
    html += "<select name='queue' required>";
    const char* queues[] = {"1.1", "1.2", "2.1", "2.2", "3.1", "3.2",
                            "4.1", "4.2", "5.1", "5.2", "6.1", "6.2"};
    for (int i = 0; i < 12; i++) {
        html += "<option value='" + String(queues[i]) + "'";
        if (String(cfg.light_queue) == String(queues[i])) {
            html += " selected";
        }
        html += ">Queue " + String(queues[i]) + "</option>";
    }
    html += "</select>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Check Interval (seconds):</label>";
    html += "<input type='number' name='interval' value='" + String(cfg.light_check_interval) + "' required>";
    html += "</div>";

    html += "<button type='submit'>Save Light Settings</button>";
    html += "</form>";

    html += generateFooter();
    server.send(200, "text/html", html);
}

void WebConfigManager::handleRGBConfig() {
    AlertLightConfig& cfg = configManager.getConfig();

    String html = generateHeader("RGB LED Configuration");
    html += "<h2>RGB LED Settings</h2>";
    html += "<form action='/save_rgb' method='POST'>";

    html += "<h3>Ambient Colors</h3>";

    html += "<div class='form-group'>";
    html += "<label>Ambient Brightness (0-100%):</label>";
    html += "<input type='number' name='ambient_brightness' min='0' max='100' value='" + String(cfg.ambient_brightness) + "' required>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>No Alert Color:</label>";
    html += "<input type='color' name='color_no_alert' value='" + colorToHex(cfg.color_no_alert) + "'>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Alert Color:</label>";
    html += "<input type='color' name='color_alert' value='" + colorToHex(cfg.color_alert) + "'>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Outage Color:</label>";
    html += "<input type='color' name='color_outage' value='" + colorToHex(cfg.color_outage) + "'>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>No Status Color:</label>";
    html += "<input type='color' name='color_no_status' value='" + colorToHex(cfg.color_no_status) + "'>";
    html += "</div>";

    html += "<h3>Blink Settings</h3>";

    html += "<div class='form-group'>";
    html += "<label>Blink ON Duration (ms):</label>";
    html += "<input type='number' name='blink_on' value='" + String(cfg.blink_on_duration) + "' required>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Blink OFF Duration (ms):</label>";
    html += "<input type='number' name='blink_off' value='" + String(cfg.blink_off_duration) + "' required>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Total Blink Duration (seconds):</label>";
    html += "<input type='number' name='blink_total' value='" + String(cfg.blink_total_duration) + "' required>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Alert Start Blink Color (Red):</label>";
    html += "<input type='color' name='blink_alert' value='" + colorToHex(cfg.color_blink_alert) + "'>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Alert Dismiss Blink Color (Green):</label>";
    html += "<input type='color' name='blink_dismiss' value='" + colorToHex(cfg.color_blink_alert_dismiss) + "'>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Outage Start Blink Color (Blue):</label>";
    html += "<input type='color' name='blink_outage' value='" + colorToHex(cfg.color_blink_outage) + "'>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<label>Power Restore Blink Color (Yellow):</label>";
    html += "<input type='color' name='blink_restore' value='" + colorToHex(cfg.color_blink_restore) + "'>";
    html += "</div>";

    html += "<button type='submit'>Save RGB Settings</button>";
    html += "</form>";

    // RGB Test Section
    html += "<h2>RGB Test Controls</h2>";
    html += "<div class='status'>";
    html += "<p>Test RGB blink patterns and ambient modes in real-time.</p>";
    html += "<div style='margin: 10px 0;'>";
    html += "<button type='button' onclick='testRGB(\"alert_start\")' style='background: #ff0000;'>Test Alert Start (Red)</button> ";
    html += "<button type='button' onclick='testRGB(\"alert_dismiss\")' style='background: #00ff00; color: #000;'>Test Alert Dismiss (Green)</button>";
    html += "</div>";
    html += "<div style='margin: 10px 0;'>";
    html += "<button type='button' onclick='testRGB(\"outage_start\")' style='background: #0000ff;'>Test Outage Start (Blue)</button> ";
    html += "<button type='button' onclick='testRGB(\"restore\")' style='background: #ffff00; color: #000;'>Test Power Restore (Yellow)</button>";
    html += "</div>";
    html += "<div style='margin: 10px 0;'>";
    html += "<button type='button' onclick='testRGB(\"ambient\")' style='background: #666;'>Test Ambient Mode</button> ";
    html += "<button type='button' onclick='testRGB(\"exit\")' style='background: #333;'>Exit Test Mode</button>";
    html += "</div>";
    html += "<div id='testStatus' style='margin-top: 10px;'></div>";
    html += "</div>";

    // JavaScript for test buttons
    html += "<script>";
    html += "function testRGB(mode) {";
    html += "  const status = document.getElementById('testStatus');";
    html += "  let msg = '';";
    html += "  switch(mode) {";
    html += "    case 'alert_start': msg = 'Testing Alert Start (Red blink)...'; break;";
    html += "    case 'alert_dismiss': msg = 'Testing Alert Dismiss (Green blink)...'; break;";
    html += "    case 'outage_start': msg = 'Testing Outage Start (Blue blink)...'; break;";
    html += "    case 'restore': msg = 'Testing Power Restore (Yellow blink)...'; break;";
    html += "    case 'ambient': msg = 'Testing Ambient Mode...'; break;";
    html += "    case 'exit': msg = 'Exiting test mode...'; break;";
    html += "  }";
    html += "  status.innerHTML = '<span style=\"color: #00aaff;\">' + msg + '</span>';";
    html += "  fetch('/test_rgb?mode=' + mode).then(r => r.text()).then(data => {";
    html += "    status.innerHTML = '<span class=\"success\">' + data + '</span>';";
    html += "  }).catch(err => {";
    html += "    status.innerHTML = '<span class=\"error\">Test failed: ' + err + '</span>';";
    html += "  });";
    html += "}";
    html += "</script>";

    html += generateFooter();
    server.send(200, "text/html", html);
}

void WebConfigManager::handleSaveWiFi() {
    AlertLightConfig& cfg = configManager.getConfig();

    if (server.hasArg("ssid")) {
        strncpy(cfg.wifi_ssid, server.arg("ssid").c_str(), sizeof(cfg.wifi_ssid) - 1);
    }
    if (server.hasArg("password")) {
        strncpy(cfg.wifi_password, server.arg("password").c_str(), sizeof(cfg.wifi_password) - 1);
    }
    cfg.use_static_ip = server.hasArg("use_static");
    if (server.hasArg("static_ip")) {
        strncpy(cfg.static_ip, server.arg("static_ip").c_str(), sizeof(cfg.static_ip) - 1);
    }
    if (server.hasArg("gateway")) {
        strncpy(cfg.static_gateway, server.arg("gateway").c_str(), sizeof(cfg.static_gateway) - 1);
    }
    if (server.hasArg("subnet")) {
        strncpy(cfg.static_subnet, server.arg("subnet").c_str(), sizeof(cfg.static_subnet) - 1);
    }

    configManager.save();

    String html = generateHeader("Settings Saved");
    html += "<h2 class='success'>WiFi Settings Saved!</h2>";
    html += "<p>The device will restart to apply the new settings.</p>";
    html += "<p><a href='/'>Return to Home</a></p>";
    html += generateFooter();

    server.send(200, "text/html", html);

    delay(2000);
    ESP.restart();
}

void WebConfigManager::handleSaveAlert() {
    AlertLightConfig& cfg = configManager.getConfig();

    if (server.hasArg("api_url")) {
        strncpy(cfg.alert_api_url, server.arg("api_url").c_str(), sizeof(cfg.alert_api_url) - 1);
    }
    if (server.hasArg("region_id")) {
        cfg.alert_region_id = server.arg("region_id").toInt();
    }
    if (server.hasArg("interval")) {
        cfg.alert_check_interval = server.arg("interval").toInt();
    }

    configManager.save();

    // Force immediate check to detect state change and trigger blink
    alertManager.forceCheck();

    String html = generateHeader("Settings Saved");
    html += "<h2 class='success'>Alert Settings Saved!</h2>";
    html += "<p><a href='/alert'>Back to Alert Settings</a></p>";
    html += generateFooter();

    server.send(200, "text/html", html);
}

void WebConfigManager::handleSaveLight() {
    AlertLightConfig& cfg = configManager.getConfig();

    if (server.hasArg("api_url")) {
        strncpy(cfg.light_api_url, server.arg("api_url").c_str(), sizeof(cfg.light_api_url) - 1);
    }
    if (server.hasArg("queue")) {
        strncpy(cfg.light_queue, server.arg("queue").c_str(), sizeof(cfg.light_queue) - 1);
    }
    if (server.hasArg("interval")) {
        cfg.light_check_interval = server.arg("interval").toInt();
    }

    configManager.save();

    String html = generateHeader("Settings Saved");
    html += "<h2 class='success'>Light Outage Settings Saved!</h2>";
    html += "<p><a href='/light'>Back to Light Settings</a></p>";
    html += generateFooter();

    server.send(200, "text/html", html);
}

void WebConfigManager::handleSaveRGB() {
    AlertLightConfig& cfg = configManager.getConfig();

    if (server.hasArg("ambient_brightness")) {
        cfg.ambient_brightness = server.arg("ambient_brightness").toInt();
    }
    if (server.hasArg("color_no_alert")) {
        cfg.color_no_alert = hexToColor(server.arg("color_no_alert"));
    }
    if (server.hasArg("color_alert")) {
        cfg.color_alert = hexToColor(server.arg("color_alert"));
    }
    if (server.hasArg("color_outage")) {
        cfg.color_outage = hexToColor(server.arg("color_outage"));
    }
    if (server.hasArg("color_no_status")) {
        cfg.color_no_status = hexToColor(server.arg("color_no_status"));
    }
    if (server.hasArg("blink_on")) {
        cfg.blink_on_duration = server.arg("blink_on").toInt();
    }
    if (server.hasArg("blink_off")) {
        cfg.blink_off_duration = server.arg("blink_off").toInt();
    }
    if (server.hasArg("blink_total")) {
        cfg.blink_total_duration = server.arg("blink_total").toInt();
    }
    if (server.hasArg("blink_alert")) {
        cfg.color_blink_alert = hexToColor(server.arg("blink_alert"));
    }
    if (server.hasArg("blink_dismiss")) {
        cfg.color_blink_alert_dismiss = hexToColor(server.arg("blink_dismiss"));
    }
    if (server.hasArg("blink_outage")) {
        cfg.color_blink_outage = hexToColor(server.arg("blink_outage"));
    }
    if (server.hasArg("blink_restore")) {
        cfg.color_blink_restore = hexToColor(server.arg("blink_restore"));
    }

    configManager.save();

    String html = generateHeader("Settings Saved");
    html += "<h2 class='success'>RGB LED Settings Saved!</h2>";
    html += "<p><a href='/rgb'>Back to RGB Settings</a></p>";
    html += generateFooter();

    server.send(200, "text/html", html);
}

void WebConfigManager::handleRestart() {
    String html = generateHeader("Restart");
    html += "<h2>Restarting Device...</h2>";
    html += "<p>The device will restart in a few seconds.</p>";
    html += generateFooter();

    server.send(200, "text/html", html);

    delay(2000);
    ESP.restart();
}

void WebConfigManager::handleNotFound() {
    if (apMode) {
        // Redirect to root for captive portal
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "");
    } else {
        String html = generateHeader("404 Not Found");
        html += "<h2 class='error'>Page Not Found</h2>";
        html += "<p><a href='/'>Return to Home</a></p>";
        html += generateFooter();

        server.send(404, "text/html", html);
    }
}

// Helper Functions

void WebConfigManager::addLog(const String& message) {
    // Get current timestamp
    unsigned long uptime = millis() / 1000;
    unsigned long hours = uptime / 3600;
    unsigned long minutes = (uptime % 3600) / 60;
    unsigned long seconds = uptime % 60;

    char timestamp[16];
    snprintf(timestamp, sizeof(timestamp), "[%02lu:%02lu:%02lu] ", hours, minutes, seconds);

    // Add to log with timestamp
    String logEntry = String(timestamp) + message + "\n";
    statusLog += logEntry;

    // Keep only last 2000 characters
    if (statusLog.length() > 2000) {
        statusLog = statusLog.substring(statusLog.length() - 2000);
    }

    printf("%s", logEntry.c_str());

    // Also add to boot screen if active
    if (AlertLight_UI_IsBootScreenActive()) {
        AlertLight_UI_AddBootLog(message.c_str());
    }
}

void WebConfigManager::checkWiFiStatus() {
    AlertLightConfig& cfg = configManager.getConfig();
    wl_status_t status = WiFi.status();

    // Check if we're in AP mode and should try to reconnect to saved WiFi
    if (apMode) {
        // In AP mode, periodically try to reconnect to saved WiFi
        if (strlen(cfg.wifi_ssid) > 0) {
            unsigned long now = millis();

            // Try to reconnect every 60 seconds
            if (now - lastScanAttempt >= scanRetryInterval) {
                addLog("Attempting to reconnect to WiFi...");

                if (connectToWiFi(15000)) {
                    // Success! Switch from AP mode to station mode
                    addLog("WiFi reconnected! Switching to Station mode");

                    WiFi.softAPdisconnect(true);  // Stop AP mode
                    apMode = false;
                    wifiConnected = true;

                    // Update UI
                    AlertLight_UI_Update_WiFi_Blink(false);  // Stop blinking
                } else {
                    addLog("Still not connected. Staying in AP mode");
                }
            }
        }
        return;
    }

    // Station mode - check connection status
    if (status == WL_CONNECTED) {
        if (!wifiConnected) {
            wifiConnected = true;
            addLog("WiFi reconnected - IP: " + WiFi.localIP().toString());
        }
    } else {
        if (wifiConnected) {
            wifiConnected = false;
            addLog("WiFi connection lost - Status: " + String(status));
        }

        // Periodic scan and reconnect attempt
        if (strlen(cfg.wifi_ssid) > 0) {
            unsigned long now = millis();

            // Only attempt reconnect/scan every scanRetryInterval (60 seconds)
            if (now - lastScanAttempt >= scanRetryInterval) {
                addLog("Attempting to reconnect to WiFi...");

                if (connectToWiFi(15000)) {
                    wifiConnected = true;
                    addLog("Successfully reconnected!");
                } else {
                    addLog("Reconnection failed. Will retry in 60s");
                }
            }
        }
    }
}

void WebConfigManager::handleStatus() {
    String html = generateHeader("Status & Logs");

    html += "<h2>System Status</h2>";
    html += "<div class='status'>";
    html += "<p><strong>Mode:</strong> " + String(apMode ? "Access Point" : "Station") + "</p>";
    html += "<p><strong>WiFi Status:</strong> ";
    if (apMode) {
        html += "AP Mode - SSID: " + apSSID + "</p>";
        html += "<p><strong>AP IP:</strong> " + WiFi.softAPIP().toString() + "</p>";
        html += "<p><strong>Clients:</strong> " + String(WiFi.softAPgetStationNum()) + "</p>";
    } else {
        if (wifiConnected && WiFi.status() == WL_CONNECTED) {
            html += "<span class='success'>Connected</span></p>";
            html += "<p><strong>SSID:</strong> " + WiFi.SSID() + "</p>";
            html += "<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>";
            html += "<p><strong>Signal:</strong> " + String(WiFi.RSSI()) + " dBm</p>";
        } else {
            html += "<span class='error'>Disconnected</span></p>";
        }
    }
    html += "<p><strong>Uptime:</strong> ";
    unsigned long uptime = millis() / 1000;
    unsigned long hours = uptime / 3600;
    unsigned long minutes = (uptime % 3600) / 60;
    unsigned long seconds = uptime % 60;
    char uptimeStr[32];
    snprintf(uptimeStr, sizeof(uptimeStr), "%luh %lum %lus", hours, minutes, seconds);
    html += String(uptimeStr) + "</p>";
    html += "<p><strong>Free Heap:</strong> " + String(ESP.getFreeHeap()) + " bytes</p>";
    html += "</div>";

    html += "<h2>System Logs</h2>";
    html += "<div class='status' style='font-family: monospace; white-space: pre-wrap; max-height: 400px; overflow-y: auto;'>";
    html += statusLog.length() > 0 ? statusLog : "No logs available";
    html += "</div>";

    html += "<script>setTimeout(function(){location.reload();}, 10000);</script>";
    html += generateFooter();

    server.send(200, "text/html", html);
}

void WebConfigManager::handleScan() {
    int16_t scanResult = WiFi.scanNetworks(true, true, true, 1500);

    if (scanResult == WIFI_SCAN_FAILED) {
        server.send(500, "application/json", "{\"error\":\"Scan failed to start\"}");
        return;
    }

    unsigned long scanStart = millis();
    int networks = 0;
    while ((networks = WiFi.scanComplete()) == WIFI_SCAN_RUNNING) {
        delay(500);
        if (millis() - scanStart > 30000) {
            WiFi.scanDelete();
            server.send(500, "application/json", "{\"error\":\"Scan timeout\"}");
            return;
        }
    }

    if (networks < 0) {
        server.send(500, "application/json", "{\"error\":\"Scan failed\"}");
        return;
    }

    // Deduplicate networks by BSSID (combine hidden networks with same BSSID)
    struct NetworkInfo {
        String ssid;
        String bssid;
        int8_t rssi;
        String encryption;
    };

    NetworkInfo uniqueNetworks[50];
    int uniqueCount = 0;

    for (int i = 0; i < networks && uniqueCount < 50; i++) {
        String ssid = WiFi.SSID(i);
        String bssid = WiFi.BSSIDstr(i);
        int8_t rssi = WiFi.RSSI(i);
        wifi_auth_mode_t encryption = WiFi.encryptionType(i);

        // Find if BSSID already exists
        int existingIndex = -1;
        for (int j = 0; j < uniqueCount; j++) {
            if (uniqueNetworks[j].bssid == bssid) {
                existingIndex = j;
                break;
            }
        }

        const char* encType = "Unknown";
        switch (encryption) {
            case WIFI_AUTH_OPEN: encType = "Open"; break;
            case WIFI_AUTH_WEP: encType = "WEP"; break;
            case WIFI_AUTH_WPA_PSK: encType = "WPA"; break;
            case WIFI_AUTH_WPA2_PSK: encType = "WPA2"; break;
            case WIFI_AUTH_WPA_WPA2_PSK: encType = "WPA/WPA2"; break;
            case WIFI_AUTH_WPA2_ENTERPRISE: encType = "WPA2-Enterprise"; break;
            default: encType = "Unknown"; break;
        }

        if (existingIndex >= 0) {
            // BSSID exists - update if this has better signal or non-empty SSID
            if (rssi > uniqueNetworks[existingIndex].rssi ||
                (ssid.length() > 0 && uniqueNetworks[existingIndex].ssid.length() == 0)) {
                if (ssid.length() > 0) {
                    uniqueNetworks[existingIndex].ssid = ssid;
                }
                uniqueNetworks[existingIndex].rssi = rssi;
                uniqueNetworks[existingIndex].encryption = String(encType);
            }
        } else {
            // New BSSID - add to list
            if (ssid.length() == 0) {
                ssid = "[Hidden Network]";
            }
            uniqueNetworks[uniqueCount].ssid = ssid;
            uniqueNetworks[uniqueCount].bssid = bssid;
            uniqueNetworks[uniqueCount].rssi = rssi;
            uniqueNetworks[uniqueCount].encryption = String(encType);
            uniqueCount++;
        }
    }

    // Build JSON response from deduplicated list
    String json = "{\"networks\":[";

    for (int i = 0; i < uniqueCount; i++) {
        if (i > 0) json += ",";

        // Escape quotes in SSID
        String ssid = uniqueNetworks[i].ssid;
        ssid.replace("\"", "\\\"");

        json += "{";
        json += "\"ssid\":\"" + ssid + "\",";
        json += "\"bssid\":\"" + uniqueNetworks[i].bssid + "\",";
        json += "\"rssi\":" + String(uniqueNetworks[i].rssi) + ",";
        json += "\"encryption\":\"" + uniqueNetworks[i].encryption + "\"";
        json += "}";
    }

    json += "],\"count\":" + String(uniqueCount) + "}";

    WiFi.scanDelete();

    server.send(200, "application/json", json);
}

void WebConfigManager::handleTestAlert() {
    alertManager.forceCheck();
    server.send(200, "text/plain", "Alert API test triggered");
}

void WebConfigManager::handleTestLight() {
    lightManager.forceCheck();
    server.send(200, "text/plain", "Light outage API test triggered");
}

void WebConfigManager::handleTestRGB() {
    String mode = server.arg("mode");
    String response;

    if (mode == "alert_start") {
        rgbManager.testBlinkAlertStart();
        response = "Testing Alert Start (Red blink for 30s)";
    } else if (mode == "alert_dismiss") {
        rgbManager.testBlinkAlertDismiss();
        response = "Testing Alert Dismiss (Green blink for 30s)";
    } else if (mode == "outage_start") {
        rgbManager.testBlinkOutageStart();
        response = "Testing Outage Start (Blue blink for 30s)";
    } else if (mode == "restore") {
        rgbManager.testBlinkRestore();
        response = "Testing Power Restore (Yellow blink for 30s)";
    } else if (mode == "ambient") {
        rgbManager.testAmbient();
        response = "Testing Ambient Mode";
    } else if (mode == "exit") {
        rgbManager.setTestMode(false);
        response = "Exited test mode - returned to normal operation";
    } else {
        response = "Unknown test mode";
    }

    server.send(200, "text/plain", response);
}

void WebConfigManager::handleNTP() {
    String html = generateHeader("Time & NTP Configuration");

    html += "<h2>Current Time</h2>";
    html += "<div class='status'>";

    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    // Check if time is valid (not epoch 0)
    if (now < 1000000000) {
        html += "<p><strong>Status:</strong> <span class='error'>Time not synchronized</span></p>";
        html += "<p><strong>Current Time:</strong> --:--:-- (invalid)</p>";
        html += "<p class='error'>Please sync time with NTP server</p>";
    } else {
        html += "<p><strong>Status:</strong> <span class='success'>Time synchronized</span></p>";

        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        html += "<p><strong>Current Time:</strong> " + String(timeStr) + "</p>";
        html += "<p><strong>Unix Timestamp:</strong> " + String(now) + "</p>";

        // Calculate uptime since last sync
        unsigned long uptime = millis() / 1000;
        unsigned long hours = uptime / 3600;
        unsigned long minutes = (uptime % 3600) / 60;
        char uptimeStr[32];
        snprintf(uptimeStr, sizeof(uptimeStr), "%luh %lum", hours, minutes);
        html += "<p><strong>System Uptime:</strong> " + String(uptimeStr) + "</p>";
    }

    html += "</div>";

    html += "<h2>NTP Configuration</h2>";
    html += "<div class='form-group'>";
    html += "<p><strong>NTP Servers:</strong> pool.ntp.org, time.nist.gov</p>";
    html += "<p><strong>Timezone:</strong> UTC+2 (Kyiv/Eastern European Time)</p>";
    html += "<p><strong>DST Offset:</strong> +1 hour (when active)</p>";
    html += "</div>";

    html += "<div class='form-group'>";
    html += "<button type='button' onclick='syncNow()' id='syncBtn'>Sync Time Now</button>";
    html += "<div id='syncStatus' style='margin-top: 10px;'></div>";
    html += "</div>";

    // JavaScript for sync button
    html += "<script>";
    html += "function syncNow() {";
    html += "  const btn = document.getElementById('syncBtn');";
    html += "  const status = document.getElementById('syncStatus');";
    html += "  btn.disabled = true;";
    html += "  status.innerHTML = '<span style=\"color: #00aaff;\">Syncing time...</span>';";
    html += "  fetch('/sync_ntp').then(r => r.text()).then(data => {";
    html += "    status.innerHTML = '<span class=\"success\">Time synchronized! Reloading in 2 seconds...</span>';";
    html += "    setTimeout(() => location.reload(), 2000);";
    html += "  }).catch(err => {";
    html += "    status.innerHTML = '<span class=\"error\">Sync failed: ' + err + '</span>';";
    html += "    btn.disabled = false;";
    html += "  });";
    html += "}";
    html += "</script>";

    html += generateFooter();
    server.send(200, "text/html", html);
}

void WebConfigManager::handleSyncNTP() {
    syncNTPTime();
    server.send(200, "text/plain", "NTP sync triggered");
}

void WebConfigManager::syncNTPTime() {
    printf("\n=== NTP Time Sync ===\n");
    configTime(2 * 3600, 3600, "pool.ntp.org", "time.nist.gov");

    int retry = 0;
    const int retry_count = 20;
    struct tm timeinfo;

    while (!getLocalTime(&timeinfo) && retry < retry_count) {
        delay(500);
        retry++;
    }

    if (retry < retry_count) {
        time_t now = time(nullptr);
        printf("SUCCESS: Time synced: %02d:%02d:%02d (Unix: %ld)\n",
               timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, now);
        addLog("Time synchronized: " + String(timeinfo.tm_hour) + ":" +
               String(timeinfo.tm_min));

        // Force light manager to recheck schedule with correct time
        lightManager.forceUpdate();
        printf("Triggered light schedule update after NTP sync\n");
    } else {
        printf("FAILED: NTP sync timeout\n");
        addLog("WARNING: NTP sync failed");
    }
    printf("=====================\n\n");
}

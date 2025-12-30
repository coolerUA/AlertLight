#include "Config.h"

ConfigManager configManager;

ConfigManager::ConfigManager() {
}

void ConfigManager::begin() {
    preferences.begin("alertlight", false);  // false = read/write mode

    // Try to load existing config
    if (!load()) {
        // No saved config, use defaults
        resetToDefaults();
        save();
    }
}

void ConfigManager::setDefaults() {
    // WiFi defaults
    strcpy(config.wifi_ssid, "");
    strcpy(config.wifi_password, "");
    config.use_static_ip = false;
    strcpy(config.static_ip, "192.168.1.100");
    strcpy(config.static_gateway, "192.168.1.1");
    strcpy(config.static_subnet, "255.255.255.0");

    // Web server defaults
    config.web_port = 8080;

    // Alert API defaults
    strcpy(config.alert_api_url, "https://air-save.ops.ajax.systems/api/mobile/status/regions/v2?regions=");
    config.alert_region_id = 16;  // Kyiv
    config.alert_check_interval = 30;  // 5 minutes

    // Light outage API defaults
    strcpy(config.light_api_url, "https://app.yasno.ua/api/blackout-service/public/shutdowns/regions/25/dsos/902/planned-outages");
    strcpy(config.light_queue, "6.2");
    config.light_check_interval = 900;  // 15 minutes

    // RGB LED defaults
    config.ambient_brightness = 10;  // 10%
    config.color_no_alert = 0x00FF00;  // Green
    config.color_alert = 0xFF0000;     // Red
    config.color_outage = 0x0000FF;    // Dark blue
    config.color_no_status = 0x808080; // Grey
    config.blink_on_duration = 500;
    config.blink_off_duration = 500;
    config.blink_total_duration = 30;
    config.color_blink_alert = 0xFF0000;    // Red
    config.color_blink_alert_dismiss = 0x00FF00;  // Green
    config.color_blink_outage = 0x00008B;   // Dark blue
    config.color_blink_restore = 0xFFFF00;  // Yellow

    // Display defaults
    config.display_brightness = 90;  // 90%
}

bool ConfigManager::load() {
    // Check if config exists
    if (!preferences.isKey("initialized")) {
        return false;
    }

    // Load WiFi settings
    preferences.getString("wifi_ssid", config.wifi_ssid, sizeof(config.wifi_ssid));
    preferences.getString("wifi_pass", config.wifi_password, sizeof(config.wifi_password));
    config.use_static_ip = preferences.getBool("use_static_ip", false);
    preferences.getString("static_ip", config.static_ip, sizeof(config.static_ip));
    preferences.getString("static_gw", config.static_gateway, sizeof(config.static_gateway));
    preferences.getString("static_sn", config.static_subnet, sizeof(config.static_subnet));

    // Load web server settings
    config.web_port = preferences.getUShort("web_port", 8080);

    // Load Alert API settings
    preferences.getString("alert_url", config.alert_api_url, sizeof(config.alert_api_url));
    config.alert_region_id = preferences.getUShort("alert_region", 16);
    config.alert_check_interval = preferences.getUInt("alert_interval", 300);

    // Load Light outage API settings
    preferences.getString("light_url", config.light_api_url, sizeof(config.light_api_url));
    preferences.getString("light_queue", config.light_queue, sizeof(config.light_queue));
    config.light_check_interval = preferences.getUInt("light_interval", 900);

    // Load RGB LED settings
    config.ambient_brightness = preferences.getUChar("rgb_ambient", 10);
    config.color_no_alert = preferences.getUInt("rgb_no_alert", 0x00FF00);
    config.color_alert = preferences.getUInt("rgb_alert", 0xFF0000);
    config.color_outage = preferences.getUInt("rgb_outage", 0x0000FF);
    config.color_no_status = preferences.getUInt("rgb_no_status", 0x808080);
    config.blink_on_duration = preferences.getUShort("blink_on", 500);
    config.blink_off_duration = preferences.getUShort("blink_off", 500);
    config.blink_total_duration = preferences.getUShort("blink_total", 30);
    config.color_blink_alert = preferences.getUInt("blink_alert", 0xFF0000);
    config.color_blink_alert_dismiss = preferences.getUInt("blink_dismiss", 0x00FF00);
    config.color_blink_outage = preferences.getUInt("blink_outage", 0x00008B);
    config.color_blink_restore = preferences.getUInt("blink_restore", 0xFFFF00);

    // Load display settings
    config.display_brightness = preferences.getUChar("disp_bright", 90);

    return true;
}

bool ConfigManager::save() {
    // Mark as initialized
    preferences.putBool("initialized", true);

    // Save WiFi settings
    preferences.putString("wifi_ssid", config.wifi_ssid);
    preferences.putString("wifi_pass", config.wifi_password);
    preferences.putBool("use_static_ip", config.use_static_ip);
    preferences.putString("static_ip", config.static_ip);
    preferences.putString("static_gw", config.static_gateway);
    preferences.putString("static_sn", config.static_subnet);

    // Save web server settings
    preferences.putUShort("web_port", config.web_port);

    // Save Alert API settings
    preferences.putString("alert_url", config.alert_api_url);
    preferences.putUShort("alert_region", config.alert_region_id);
    preferences.putUInt("alert_interval", config.alert_check_interval);

    // Save Light outage API settings
    preferences.putString("light_url", config.light_api_url);
    preferences.putString("light_queue", config.light_queue);
    preferences.putUInt("light_interval", config.light_check_interval);

    // Save RGB LED settings
    preferences.putUChar("rgb_ambient", config.ambient_brightness);
    preferences.putUInt("rgb_no_alert", config.color_no_alert);
    preferences.putUInt("rgb_alert", config.color_alert);
    preferences.putUInt("rgb_outage", config.color_outage);
    preferences.putUInt("rgb_no_status", config.color_no_status);
    preferences.putUShort("blink_on", config.blink_on_duration);
    preferences.putUShort("blink_off", config.blink_off_duration);
    preferences.putUShort("blink_total", config.blink_total_duration);
    preferences.putUInt("blink_alert", config.color_blink_alert);
    preferences.putUInt("blink_dismiss", config.color_blink_alert_dismiss);
    preferences.putUInt("blink_outage", config.color_blink_outage);
    preferences.putUInt("blink_restore", config.color_blink_restore);

    // Save display settings
    preferences.putUChar("disp_bright", config.display_brightness);

    return true;
}

void ConfigManager::resetToDefaults() {
    setDefaults();
}

AlertLightConfig& ConfigManager::getConfig() {
    return config;
}

void ConfigManager::setWiFiCredentials(const char* ssid, const char* password) {
    strncpy(config.wifi_ssid, ssid, sizeof(config.wifi_ssid) - 1);
    strncpy(config.wifi_password, password, sizeof(config.wifi_password) - 1);
}

void ConfigManager::setStaticIP(const char* ip, const char* gateway, const char* subnet) {
    config.use_static_ip = true;
    strncpy(config.static_ip, ip, sizeof(config.static_ip) - 1);
    strncpy(config.static_gateway, gateway, sizeof(config.static_gateway) - 1);
    strncpy(config.static_subnet, subnet, sizeof(config.static_subnet) - 1);
}

void ConfigManager::setAlertAPI(const char* url, uint16_t region_id, uint32_t interval) {
    strncpy(config.alert_api_url, url, sizeof(config.alert_api_url) - 1);
    config.alert_region_id = region_id;
    config.alert_check_interval = interval;
}

void ConfigManager::setLightAPI(const char* url, const char* queue, uint32_t interval) {
    strncpy(config.light_api_url, url, sizeof(config.light_api_url) - 1);
    strncpy(config.light_queue, queue, sizeof(config.light_queue) - 1);
    config.light_check_interval = interval;
}

void ConfigManager::setRGBColors(uint32_t no_alert, uint32_t alert, uint32_t outage, uint32_t no_status) {
    config.color_no_alert = no_alert;
    config.color_alert = alert;
    config.color_outage = outage;
    config.color_no_status = no_status;
}

void ConfigManager::setRGBBlinkSettings(uint16_t on_ms, uint16_t off_ms, uint16_t total_sec) {
    config.blink_on_duration = on_ms;
    config.blink_off_duration = off_ms;
    config.blink_total_duration = total_sec;
}

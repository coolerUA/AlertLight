#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

// Configuration structure
struct AlertLightConfig {
    // WiFi Settings
    char wifi_ssid[32];
    char wifi_password[64];
    bool use_static_ip;
    char static_ip[16];
    char static_gateway[16];
    char static_subnet[16];

    // Web Server Settings
    uint16_t web_port;

    // Alert API Settings
    char alert_api_url[128];
    uint16_t alert_region_id;
    uint32_t alert_check_interval;  // seconds

    // Light Outage API Settings
    char light_api_url[128];
    char light_queue[8];
    uint32_t light_check_interval;  // seconds

    // Yasno address selection (persisted for display and reuse)
    uint32_t yasno_street_id;
    char yasno_street_name[64];
    uint32_t yasno_house_id;
    char yasno_house_name[16];

    // RGB LED Settings
    uint8_t ambient_brightness;     // 0-100%
    uint32_t color_no_alert;        // RGB hex color
    uint32_t color_alert;
    uint32_t color_outage;
    uint32_t color_no_status;
    uint16_t blink_on_duration;     // milliseconds
    uint16_t blink_off_duration;    // milliseconds
    uint16_t blink_total_duration;  // seconds
    uint32_t color_blink_alert;
    uint32_t color_blink_alert_dismiss;  // Green blink when alert dismissed
    uint32_t color_blink_outage;
    uint32_t color_blink_restore;

    // Display Settings
    uint8_t display_brightness;     // 0-100%
};

class ConfigManager {
public:
    ConfigManager();

    // Initialize with default values
    void begin();

    // Load configuration from NVS
    bool load();

    // Save configuration to NVS
    bool save();

    // Reset to factory defaults
    void resetToDefaults();

    // Get current configuration
    AlertLightConfig& getConfig();

    // Update individual settings
    void setWiFiCredentials(const char* ssid, const char* password);
    void setStaticIP(const char* ip, const char* gateway, const char* subnet);
    void setAlertAPI(const char* url, uint16_t region_id, uint32_t interval);
    void setLightAPI(const char* url, const char* queue, uint32_t interval);
    void setRGBColors(uint32_t no_alert, uint32_t alert, uint32_t outage, uint32_t no_status);
    void setRGBBlinkSettings(uint16_t on_ms, uint16_t off_ms, uint16_t total_sec);

private:
    Preferences preferences;
    AlertLightConfig config;

    void setDefaults();
};

extern ConfigManager configManager;

#endif // CONFIG_H

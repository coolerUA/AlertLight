/**
 ******************************************************************************
 * @file     LVGL_Arduino.ino
 * @author   Yongqin Ou
 * @version  V1.0
 * @date     2024-10-30
 * @brief    Setup experiment for multiple modules
 * @license  MIT
 * @copyright Copyright (c) 2024, Waveshare
 ******************************************************************************
 *
 * Experiment Objective: Learn how to set up and use multiple modules including SD card, display, LVGL, wireless, and RGB lamp.
 *
 * Hardware Resources and Pin Assignment:
 * 1. SD Card Interface --> As configured in SD_Card.h.
 * 2. Display Interface --> As configured in Display_ST7789.h.
 * 3. Wireless Module Interface --> As configured in Wireless.h.
 * 4. RGB Lamp Interface --> As configured in RGB_lamp.h.
 *
 * Experiment Phenomenon:
 * 1. Runs various tests and initializations for different modules.
 * 2. Displays LVGL examples on the display.
 * 3. Continuously runs loops for timer, RGB lamp, and other tasks.
 *
 * Notes:
 * None
 *
 ******************************************************************************
 *
 * Development Platform: ESP32
 * Support Forum: service.waveshare.com
 * Company Website: www.waveshare.com
 *
 ******************************************************************************
 */
#include "src/SD_Card/SD_Card.h"
#include "src/Display/Display_ST7789.h"
#include "src/LVGL_Driver/LVGL_Driver.h"
#include "src/Wireless/Wireless.h"
#include "src/RGB_Lamp/RGB_lamp.h"
#include "src/AlertLight_UI/AlertLight_UI.h"
#include "src/Config/Config.h"
#include "src/WebConfig/WebConfig.h"
#include "src/AlertManager/AlertManager.h"
#include "src/LightManager/LightManager.h"
#include "src/RGBManager/RGBManager.h"

void setup()
{
  // Initialize Serial for debugging (ESP32-S3 USB-JTAG)
  Serial.begin(115200);
  delay(100);  // Short delay for serial to stabilize

  Flash_test();
  printf("After Flash_test\n");

  SD_Init();
  printf("After SD_Init\n");

  printf("\n========== AlertLight Starting ==========\n");
  printf("ESP32-S3 AlertLight v1.0\n");
  printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  printf("=========================================\n\n");

  // Initialize configuration system
  printf("Initializing configuration...\n");
  configManager.begin();
  printf("Config initialized\n");

  // Initialize display and UI
  printf("Initializing display...\n");
  LCD_Init();
  printf("LCD OK\n");

  Set_Backlight(configManager.getConfig().display_brightness);
  printf("Backlight OK\n");

  Lvgl_Init();
  printf("LVGL OK\n");

  AlertLight_UI_Init();
  printf("UI OK\n");

  // Let LVGL complete initialization before changing screens
  lv_timer_handler();
  delay(10);
  lv_timer_handler();

  // Add boot logs (will be displayed when boot screen loads)
  AlertLight_UI_AddBootLog("System initialized");
  AlertLight_UI_AddBootLog("Display OK");

  // Initialize WiFi and web server
  AlertLight_UI_AddBootLog("Starting WiFi...");
  AlertLight_UI_Update_WiFi_Blink(true);  // Start blinking

  webConfig.begin();
  printf("WiFi manager started\n");

  // Initialize Alert Manager
  alertManager.begin();
  printf("Alert Manager initialized\n");

  // Initialize Light Outage Manager
  lightManager.begin();
  printf("Light Outage Manager initialized\n");

  // Initialize RGB Manager
  rgbManager.begin();
  printf("RGB Manager initialized\n");

  // Boot screen will be shown for 10 seconds, hiding in loop
  printf("\n========== Setup Complete ==========\n");
  printf("Entering main loop...\n");
}

void loop()
{
  static unsigned long bootScreenStartTime = 0;
  static bool bootScreenShown = false;
  static bool bootScreenHidden = false;
  static bool wasConnected = false;  // Track WiFi state changes

  // Show boot screen after 5 seconds
  if (!bootScreenShown && millis() >= 5000) {
    printf("Transitioning to boot screen from loop\n");
    AlertLight_UI_ShowBootScreen();
    AlertLight_UI_Update_IP("Connecting...", "...");
    bootScreenShown = true;
    bootScreenStartTime = millis();
  }

  // Handle web server requests
  webConfig.handleClient();

  // Detect WiFi connection event (transition from disconnected to connected)
  bool isConnected = webConfig.isConnected();
  if (isConnected && !wasConnected) {
    printf("\n========== WiFi Just Connected! ==========\n");
    printf("Triggering immediate API updates...\n");

    // Force immediate updates by resetting last update times in managers
    alertManager.forceUpdate();
    lightManager.forceUpdate();

    printf("==========================================\n\n");
  }
  wasConnected = isConnected;

  // Hide boot screen after 10 seconds
  if (bootScreenShown && !bootScreenHidden && AlertLight_UI_IsBootScreenActive()) {
    unsigned long now = millis();
    if (now - bootScreenStartTime >= 10000) {  // 10 seconds
      AlertLight_UI_AddBootLog("Boot complete!");
      delay(500);  // Show final message briefly

      AlertLight_UI_HideBootScreen();
      AlertLight_UI_Update_WiFi_Blink(false);  // Stop blinking
      bootScreenHidden = true;

      // Initialize normal UI with current status
      String port = String(configManager.getConfig().web_port);
      if (webConfig.isConnected()) {
        String ip = webConfig.getIPAddress();
        AlertLight_UI_Update_IP(ip.c_str(), port.c_str());
        AlertLight_UI_Update_WiFi(UI_WIFI_CONNECTED);
      } else {
        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
          AlertLight_UI_Update_IP("192.168.4.1", port.c_str());
          AlertLight_UI_Update_WiFi(UI_WIFI_AP_MODE);
        } else {
          AlertLight_UI_Update_IP("Disconnected", "");
          AlertLight_UI_Update_WiFi(UI_WIFI_DISCONNECTED);
        }
      }

      AlertLight_UI_Update_Alert("Not configured", "Not checked yet", false);

      // Update light section with current data (or placeholder if no data yet)
      const std::vector<OutageRange>& ranges = lightManager.getOutageRanges();
      if (ranges.size() > 0) {
        // Use existing data from lightManager
        printf("Refreshing light display with existing data (%d slots)\n", ranges.size());
        outage_time_slot_t slots[ranges.size()];
        for (size_t i = 0; i < ranges.size(); i++) {
          slots[i].time_range = ranges[i].time_range.c_str();
          slots[i].is_active = ranges[i].is_active;
        }
        AlertLight_UI_Update_Light(configManager.getConfig().light_queue, slots, ranges.size());
      } else {
        // No data yet, use placeholder
        printf("No light data yet, using placeholder\n");
        outage_time_slot_t init_slot;
        init_slot.time_range = "--:--";
        init_slot.is_active = false;
        AlertLight_UI_Update_Light(configManager.getConfig().light_queue, &init_slot, 1);
      }
    }
  }

  // Update WiFi status on display periodically (only after boot screen is hidden)
  if (bootScreenHidden) {
    static unsigned long lastUIUpdate = 0;
    unsigned long now = millis();
    if (now - lastUIUpdate >= 2000) {  // Update every 2 seconds
      lastUIUpdate = now;

      // Update WiFi status and IP
      String port = String(configManager.getConfig().web_port);
      if (webConfig.isConnected()) {
        // Connected - show IP:port
        String ip = webConfig.getIPAddress();
        AlertLight_UI_Update_IP(ip.c_str(), port.c_str());
        AlertLight_UI_Update_WiFi(UI_WIFI_CONNECTED);
        AlertLight_UI_Update_WiFi_Blink(false);  // Stop blinking when connected
      } else if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        // AP mode - show AP IP with port (AP mode is stable, not connecting)
        AlertLight_UI_Update_IP("192.168.4.1", port.c_str());
        AlertLight_UI_Update_WiFi(UI_WIFI_AP_MODE);
        AlertLight_UI_Update_WiFi_Blink(false);  // Stop blinking in AP mode
      } else if (AlertLight_UI_IsWiFiBlinking()) {
        // Currently trying to connect - show "Connecting" without port
        AlertLight_UI_Update_IP("Connecting", "");
        AlertLight_UI_Update_WiFi(UI_WIFI_DISCONNECTED);
      } else {
        // Disconnected - show status without port
        AlertLight_UI_Update_IP("Disconnected", "");
        AlertLight_UI_Update_WiFi(UI_WIFI_DISCONNECTED);
      }
    }
  }

  // Update Alert Manager (handles periodic API checks)
  alertManager.update();

  // Update Light Outage Manager (handles periodic schedule checks)
  lightManager.update();

  // Update RGB Manager (handles ambient colors and blink notifications)
  rgbManager.update();

  // Update clock display and outage highlighting every second
  static unsigned long lastClockUpdate = 0;
  if (millis() - lastClockUpdate >= 1000) {
    AlertLight_UI_Update_Clock();
    lightManager.updateActiveStates();  // Recalculate which outage is active
    lastClockUpdate = millis();
  }

  Timer_Loop();
  AlertLight_UI_Tick(); // Update UI (handles blinking)
  // RGB_Lamp_Loop(2);  // Replaced by RGBManager
  delay(5);
}

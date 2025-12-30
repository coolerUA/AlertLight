#ifndef ALERTLIGHT_UI_H
#define ALERTLIGHT_UI_H

#include "../../UI_Mockup/src/ui/ui.h"
#include "../../UI_Mockup/src/ui/screens.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the AlertLight UI
void AlertLight_UI_Init(void);

// WiFi connection modes for UI
typedef enum {
    UI_WIFI_DISCONNECTED = 0,
    UI_WIFI_AP_MODE = 1,
    UI_WIFI_CONNECTED = 2
} ui_wifi_mode_t;

// Boot screen functions
void AlertLight_UI_ShowBootScreen(void);
void AlertLight_UI_AddBootLog(const char* message);
void AlertLight_UI_HideBootScreen(void);
bool AlertLight_UI_IsBootScreenActive(void);

// Update UI elements with current data
void AlertLight_UI_Update_IP(const char* ip, const char* port);
void AlertLight_UI_Update_WiFi(ui_wifi_mode_t mode);
void AlertLight_UI_Update_WiFi_Blink(bool blink);  // Enable/disable blinking
bool AlertLight_UI_IsWiFiBlinking(void);  // Get current WiFi blinking state
void AlertLight_UI_Update_Alert(const char* region, const char* status, bool is_alert);
// Structure for outage time range
typedef struct {
    const char* time_range;  // e.g., "08:30-12:30"
    bool is_active;          // true if this is the currently active outage
} outage_time_slot_t;

void AlertLight_UI_Update_Light(const char* queue, const outage_time_slot_t* slots, int num_slots);
void AlertLight_UI_Update_LightIndicator(bool is_outage);
void AlertLight_UI_Update_LightIndicator_Emergency(bool is_emergency);

// Tick function - call this in loop()
void AlertLight_UI_Tick(void);

// Clock update function - call periodically to update time display
void AlertLight_UI_Update_Clock(void);

#ifdef __cplusplus
}
#endif

#endif // ALERTLIGHT_UI_H

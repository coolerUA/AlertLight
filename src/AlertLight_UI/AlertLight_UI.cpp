#include "AlertLight_UI.h"
#include <Arduino.h>
#include <time.h>
#include "../Fonts/lv_font_montserrat_10_cyrillic.h"
// Diagnostics disabled to save flash space
// #include "../LVGL_Driver/LVGL_Diagnostics.h"

// Include all Cyrillic font sizes (using bold variants)
extern "C" {
    extern const lv_font_t lv_font_montserrat_bold_10_cyrillic;
    extern const lv_font_t lv_font_montserrat_bold_12_cyrillic;
    extern const lv_font_t lv_font_montserrat_bold_14_cyrillic;
    extern const lv_font_t lv_font_montserrat_bold_16_cyrillic;
    extern const lv_font_t lv_font_montserrat_bold_18_cyrillic;
    extern const lv_font_t lv_font_montserrat_bold_20_cyrillic;
    extern const lv_font_t lv_font_montserrat_bold_24_cyrillic;
}

// Include the generated UI source files directly since Arduino doesn't compile .c files outside the sketch
extern "C" {
    #include "../../UI_Mockup/src/ui/screens.c"
    #include "../../UI_Mockup/src/ui/images.c"
    #include "../../UI_Mockup/src/ui/styles.c"
    #include "../../UI_Mockup/src/ui/ui.c"
}

// Screen management
static unsigned long init_screen_start_time = 0;
static bool init_screen_active = false;
static bool boot_screen_active = false;
static bool main_screen_active = false;
static bool wifi_blink_enabled = false;
static unsigned long last_blink_time = 0;
static bool blink_state = false;
static String boot_log_text = "";

// Clock display
static lv_obj_t *clock_label = NULL;

// Dynamic outage time labels (created based on number of outages)
#define MAX_OUTAGE_SLOTS 6
static lv_obj_t *outage_time_labels[MAX_OUTAGE_SLOTS] = {NULL};
static int num_outage_labels = 0;

// Helper function to count UTF-8 characters (not bytes)
static int utf8_char_count(const char* str) {
    if (str == NULL) return 0;

    int count = 0;
    while (*str) {
        // Count the start of each UTF-8 character
        // UTF-8 continuation bytes start with 10xxxxxx (0x80-0xBF)
        // Only count bytes that are NOT continuation bytes
        if ((*str & 0xC0) != 0x80) {
            count++;
        }
        str++;
    }
    return count;
}

// Initialize the AlertLight UI
void AlertLight_UI_Init(void) {
    printf("\n=== AlertLight UI Initialization ===\n");

    // Verify LVGL display is ready
    if (lv_disp_get_default() == NULL) {
        printf("ERROR: No display!\n");
        return;
    }

    // Create all three screens in order
    printf("Creating init screen...\n");
    create_screen_init();
    if (objects.init == NULL) {
        printf("ERROR: Init screen creation failed!\n");
        return;
    }
    printf("Init screen created: %p\n", objects.init);

    printf("Creating boot screen...\n");
    create_screen_boot();
    if (objects.boot == NULL) {
        printf("ERROR: Boot screen creation failed!\n");
        return;
    }
    printf("Boot screen created: %p\n", objects.boot);

    printf("Creating main screen...\n");
    create_screen_main();
    if (objects.main == NULL) {
        printf("ERROR: Main screen creation failed!\n");
        return;
    }
    printf("Main screen created: %p\n", objects.main);

    // Load init screen as the starting screen
    lv_scr_load(objects.init);
    init_screen_active = true;
    init_screen_start_time = millis();
    boot_screen_active = false;
    main_screen_active = false;
    printf("Init screen loaded (will transition to boot after 5 seconds)\n");

    // Test main screen objects
    printf("\n=== Main Screen Objects ===\n");

    // Initialize IP display with "Connecting" placeholder
    if (objects.device_ip) {
        printf("device_ip: %p OK\n", objects.device_ip);
        lv_label_set_text(objects.device_ip, "Connecting");
    } else {
        printf("ERROR: device_ip is NULL!\n");
    }

    // Test WiFi indicator
    if (objects.wifi_indicator) {
        printf("wifi_indicator: %p OK\n", objects.wifi_indicator);
        lv_led_on(objects.wifi_indicator);
        lv_led_set_color(objects.wifi_indicator, lv_color_hex(0x00ff00)); // Green
    } else {
        printf("ERROR: wifi_indicator is NULL!\n");
    }

    // Hide device_port (not needed - port removed from display)
    if (objects.device_port) {
        printf("device_port: %p - hiding (not used)\n", objects.device_port);
        lv_label_set_text(objects.device_port, ""); // Clear any placeholder text
        lv_obj_add_flag(objects.device_port, LV_OBJ_FLAG_HIDDEN); // Hide the object
    }

    // Test alert section
    if (objects.region_name) {
        printf("region_name: %p OK\n", objects.region_name);
        lv_label_set_text(objects.region_name, "Not updated");
        lv_obj_set_style_text_font(objects.region_name, &lv_font_montserrat_bold_14_cyrillic, 0);
        lv_obj_set_style_text_color(objects.region_name, lv_color_hex(0xffffff), 0); // White
    } else {
        printf("ERROR: region_name is NULL!\n");
    }

    if (objects.alert_status) {
        printf("alert_status: %p OK\n", objects.alert_status);
        lv_label_set_text(objects.alert_status, "Not updated");
        lv_obj_set_style_text_font(objects.alert_status, &lv_font_montserrat_bold_14_cyrillic, 0);
        lv_obj_set_style_text_color(objects.alert_status, lv_color_hex(0xffffff), 0); // White
    } else {
        printf("ERROR: alert_status is NULL!\n");
    }

    if (objects.alert_indicator) {
        printf("alert_indicator: %p OK\n", objects.alert_indicator);
        // Initialize as OFF/grey until real data arrives
        lv_led_set_color(objects.alert_indicator, lv_color_hex(0x808080)); // Grey
        lv_led_set_brightness(objects.alert_indicator, 0); // Off
        lv_led_off(objects.alert_indicator);
    } else {
        printf("ERROR: alert_indicator is NULL!\n");
    }

    // Test light outage section
    if (objects.queue_value) {
        printf("queue_value: %p OK\n", objects.queue_value);
        lv_label_set_text(objects.queue_value, "1.1");
    } else {
        printf("ERROR: queue_value is NULL!\n");
    }

    if (objects.outage_time_1) {
        printf("outage_time_1: %p OK\n", objects.outage_time_1);
        lv_label_set_text(objects.outage_time_1, "08:00-12:00");
    } else {
        printf("ERROR: outage_time_1 is NULL!\n");
    }

    if (objects.outage_time_2) {
        printf("outage_time_2: %p OK\n", objects.outage_time_2);
        lv_label_set_text(objects.outage_time_2, "16:00-20:00");
    } else {
        printf("ERROR: outage_time_2 is NULL!\n");
    }

    if (objects.light_indicator) {
        printf("light_indicator: %p OK\n", objects.light_indicator);
        // Initialize as OFF/grey until real data arrives
        lv_led_set_color(objects.light_indicator, lv_color_hex(0x808080)); // Grey
        lv_led_set_brightness(objects.light_indicator, 0); // Off
        lv_led_off(objects.light_indicator);
    } else {
        printf("ERROR: light_indicator is NULL!\n");
    }

    // Create clock display at the bottom of the screen
    if (objects.main) {
        clock_label = lv_label_create(objects.main);
        if (clock_label) {
            lv_obj_set_pos(clock_label, 0, 282);  // Position near bottom (320-38=282)
            lv_obj_set_size(clock_label, 172, 38);  // Full width, 38px height (larger)
            lv_obj_set_style_bg_color(clock_label, lv_color_hex(0x0a0a0a), 0);  // Dark background
            lv_obj_set_style_bg_opa(clock_label, LV_OPA_COVER, 0);
            lv_obj_set_style_text_color(clock_label, lv_color_hex(0x00ff00), 0);  // Green text (like digital clock)
            lv_obj_set_style_text_align(clock_label, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_font(clock_label, &lv_font_montserrat_bold_24_cyrillic, 0);  // 24px font (largest available)
            lv_obj_set_style_pad_top(clock_label, 7, 0);  // More padding for better centering
            lv_obj_set_style_border_width(clock_label, 1, 0);  // Add subtle border
            lv_obj_set_style_border_color(clock_label, lv_color_hex(0x404040), 0);  // Dark grey border
            lv_label_set_text(clock_label, "--:--:--");
            printf("clock_label: %p OK (at Y=282, 38px height, 24px font)\n", clock_label);
        } else {
            printf("ERROR: Failed to create clock_label!\n");
        }
    }

    printf("\nTEST: Main screen active. Check all UI elements.\n");
    printf("==================================================\n\n");
}

// Update IP address and port
void AlertLight_UI_Update_IP(const char* ip, const char* port) {
    // SAFETY: Don't access objects if not initialized
    if (objects.device_ip == NULL) {
        return;
    }

    static char combined_text[32];

    // Combine IP and port into single string (device_port object hidden)
    if (ip != NULL && port != NULL && strlen(port) > 0) {
        snprintf(combined_text, sizeof(combined_text), "%s:%s", ip, port);
    } else if (ip != NULL) {
        snprintf(combined_text, sizeof(combined_text), "%s", ip);
    } else {
        return;
    }

    // Auto-scale font size based on text length to prevent overflow
    int text_len = strlen(combined_text);
    const lv_font_t* font;

    if (text_len <= 12) {
        // Short text (e.g., "Connecting", "10.0.0.1")
        font = &lv_font_montserrat_18;
    } else if (text_len <= 16) {
        // Medium text (e.g., "192.168.100.1", "Disconnected")
        font = &lv_font_montserrat_14;
    } else {
        // Long text
        font = &lv_font_montserrat_12;
    }

    lv_obj_set_style_text_font(objects.device_ip, font, 0);
    lv_label_set_text(objects.device_ip, combined_text);
}

// Update WiFi indicator
void AlertLight_UI_Update_WiFi(ui_wifi_mode_t mode) {
    // SAFETY: Don't access objects if not initialized
    if (objects.wifi_indicator == NULL) {
        return;
    }

    switch (mode) {
        case UI_WIFI_CONNECTED:
            // Green for connected to WiFi
            lv_led_on(objects.wifi_indicator);
            lv_led_set_color(objects.wifi_indicator, lv_color_hex(0x00ff00));
            lv_led_set_brightness(objects.wifi_indicator, 255);
            break;

        case UI_WIFI_AP_MODE:
            // Orange for AP mode
            lv_led_on(objects.wifi_indicator);
            lv_led_set_color(objects.wifi_indicator, lv_color_hex(0xffa500));
            lv_led_set_brightness(objects.wifi_indicator, 255);
            break;

        case UI_WIFI_DISCONNECTED:
        default:
            // LED off for disconnected
            lv_led_off(objects.wifi_indicator);
            break;
    }
}

// Update alert section
void AlertLight_UI_Update_Alert(const char* region, const char* status, bool is_alert) {
    // SAFETY: Don't access objects if not initialized
    if (objects.region_name == NULL || objects.alert_status == NULL || objects.alert_indicator == NULL) {
        return;
    }

    if (region != NULL) {
        lv_label_set_text(objects.region_name, region);

        // Auto-scale Cyrillic font based on character count (not bytes)
        // Available space is approximately 120px (between alert indicator and right border)
        int char_count = utf8_char_count(region);
        const lv_font_t* font;
        int font_size = 10;

        // Bold fonts are wider, so use more conservative thresholds
        if (char_count <= 5) {
            // Very short text - use largest bold font
            font = &lv_font_montserrat_bold_24_cyrillic;
            font_size = 24;
        } else if (char_count <= 7) {
            // Short text
            font = &lv_font_montserrat_bold_20_cyrillic;
            font_size = 20;
        } else if (char_count <= 8) {
            // Medium-short text
            font = &lv_font_montserrat_bold_18_cyrillic;
            font_size = 18;
        } else if (char_count <= 9) {
            // Medium text
            font = &lv_font_montserrat_bold_16_cyrillic;
            font_size = 16;
        } else if (char_count <= 11) {
            // Medium-long text
            font = &lv_font_montserrat_bold_14_cyrillic;
            font_size = 14;
        } else if (char_count <= 13) {
            // Long text
            font = &lv_font_montserrat_bold_12_cyrillic;
            font_size = 12;
        } else {
            // Very long text
            font = &lv_font_montserrat_bold_10_cyrillic;
            font_size = 10;
        }

        lv_obj_set_style_text_font(objects.region_name, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    if (status != NULL) {
        lv_label_set_text(objects.alert_status, status);
    }

    // Determine if this is unknown/no-data status
    bool is_unknown = false;
    if (status != NULL) {
        String statusStr = String(status);
        if (statusStr == "Not checked yet" || statusStr == "Unknown" ||
            statusStr == "No data" || statusStr == "Initializing") {
            is_unknown = true;
        }
    }
    if (region != NULL) {
        String regionStr = String(region);
        if (regionStr == "Unknown" || regionStr == "Not configured") {
            is_unknown = true;
        }
    }

    // Update indicator and text color based on alert status
    if (is_unknown) {
        // Grey for unknown/no data
        lv_led_set_color(objects.alert_indicator, lv_color_hex(0x808080));
        lv_led_set_brightness(objects.alert_indicator, 150);
        lv_led_on(objects.alert_indicator);
        lv_obj_set_style_text_color(objects.alert_status, lv_color_hex(0xaaaaaa), 0); // Brighter grey text
        lv_obj_set_style_text_opa(objects.alert_status, LV_OPA_COVER, 0); // Full opacity
        lv_obj_set_style_text_color(objects.region_name, lv_color_hex(0xaaaaaa), 0); // Brighter grey region name
        lv_obj_set_style_text_opa(objects.region_name, LV_OPA_COVER, 0); // Full opacity
    } else if (is_alert) {
        // Red for alert - use bright, highly visible red
        lv_led_set_color(objects.alert_indicator, lv_color_hex(0xff0000));
        lv_led_set_brightness(objects.alert_indicator, 255);
        lv_led_on(objects.alert_indicator);
        lv_obj_set_style_text_color(objects.alert_status, lv_color_hex(0xff3333), 0); // Brighter red text
        lv_obj_set_style_text_opa(objects.alert_status, LV_OPA_COVER, 0); // Full opacity
        lv_obj_set_style_text_color(objects.region_name, lv_color_hex(0xff3333), 0); // Brighter red region name
        lv_obj_set_style_text_opa(objects.region_name, LV_OPA_COVER, 0); // Full opacity
    } else {
        // No alert - green LED to indicate safe status
        lv_led_on(objects.alert_indicator);
        lv_led_set_color(objects.alert_indicator, lv_color_hex(0x00ff00)); // Green
        lv_led_set_brightness(objects.alert_indicator, 255); // Bright green
        lv_obj_set_style_text_color(objects.alert_status, lv_color_hex(0x33ff33), 0); // Brighter green text
        lv_obj_set_style_text_opa(objects.alert_status, LV_OPA_COVER, 0); // Full opacity
        lv_obj_set_style_text_color(objects.region_name, lv_color_hex(0x33ff33), 0); // Brighter green region name
        lv_obj_set_style_text_opa(objects.region_name, LV_OPA_COVER, 0); // Full opacity
    }
}

// Update light outage schedule with dynamic slots
void AlertLight_UI_Update_Light(const char* queue, const outage_time_slot_t* slots, int num_slots) {
    printf(">>> UI_Update_Light called: queue=%s, num_slots=%d\n", queue ? queue : "NULL", num_slots);
    for (int i = 0; i < num_slots; i++) {
        printf("    Received slot %d: '%s' (active=%d)\n", i,
               slots[i].time_range ? slots[i].time_range : "NULL",
               slots[i].is_active);
    }

    // SAFETY: Don't access objects if not initialized
    if (objects.queue_value == NULL || objects.light_section == NULL) {
        printf("ERROR: queue_value or light_section is NULL!\n");
        return;
    }

    // Update queue name
    if (queue != NULL) {
        lv_label_set_text(objects.queue_value, queue);

        // Highlight queue in yellow if any outage is active
        bool has_active = false;
        for (int i = 0; i < num_slots; i++) {
            if (slots[i].is_active) {
                has_active = true;
                break;
            }
        }

        if (has_active) {
            lv_obj_set_style_text_color(objects.queue_value, lv_color_hex(0xffff00), 0); // Yellow
        } else {
            lv_obj_set_style_text_color(objects.queue_value, lv_color_hex(0xffffff), 0); // White
        }
        lv_obj_set_style_text_opa(objects.queue_value, LV_OPA_COVER, 0);
    }

    // Hide old static labels from EEZ (we'll use dynamic ones)
    if (objects.outage_time_1) {
        lv_obj_add_flag(objects.outage_time_1, LV_OBJ_FLAG_HIDDEN);
    }
    if (objects.outage_time_2) {
        lv_obj_add_flag(objects.outage_time_2, LV_OBJ_FLAG_HIDDEN);
    }

    // Delete old dynamic labels if the number changed
    if (num_slots != num_outage_labels) {
        for (int i = 0; i < num_outage_labels; i++) {
            if (outage_time_labels[i]) {
                lv_obj_del(outage_time_labels[i]);
                outage_time_labels[i] = NULL;
            }
        }
        num_outage_labels = 0;
    }

    // Create/update dynamic outage labels
    int y_pos = 80;  // Starting Y position for outage times (below queue)
    const int line_height = 20;  // Height per line

    for (int i = 0; i < num_slots && i < MAX_OUTAGE_SLOTS; i++) {
        // Create label if it doesn't exist
        if (outage_time_labels[i] == NULL) {
            outage_time_labels[i] = lv_label_create(objects.light_section);
            lv_obj_set_pos(outage_time_labels[i], 12, y_pos);
            lv_obj_set_size(outage_time_labels[i], 148, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(outage_time_labels[i], &lv_font_montserrat_bold_14_cyrillic, 0);
        }

        // Update label text and color
        String text = String(slots[i].time_range);
        if (slots[i].is_active) {
            text += " <";  // Add marker for active slot
            lv_obj_set_style_text_color(outage_time_labels[i], lv_color_hex(0xffff00), 0); // Yellow
        } else {
            lv_obj_set_style_text_color(outage_time_labels[i], lv_color_hex(0xffffff), 0); // White
        }

        lv_label_set_text(outage_time_labels[i], text.c_str());
        printf("    Label %d set to: '%s'\n", i, text.c_str());
        lv_obj_set_style_text_opa(outage_time_labels[i], LV_OPA_COVER, 0);
        lv_obj_clear_flag(outage_time_labels[i], LV_OBJ_FLAG_HIDDEN);  // Make sure it's visible

        y_pos += line_height;
    }

    num_outage_labels = num_slots;
    printf("<<< UI_Update_Light finished: %d labels created/updated\n\n", num_outage_labels);
}

// Update light outage indicator (pass true for outage, false for no outage)
void AlertLight_UI_Update_LightIndicator(bool is_outage) {
    // SAFETY: Don't access objects if not initialized
    if (objects.light_indicator == NULL) {
        return;
    }

    if (is_outage) {
        lv_led_on(objects.light_indicator);
        lv_led_set_color(objects.light_indicator, lv_color_hex(0xf5ff00)); // Yellow
        lv_led_set_brightness(objects.light_indicator, 255);
    } else {
        // No outage - turn LED completely off with grey color and zero brightness
        lv_led_set_color(objects.light_indicator, lv_color_hex(0x808080)); // Grey
        lv_led_set_brightness(objects.light_indicator, 0); // Completely off
        lv_led_off(objects.light_indicator);
    }
}

// Update light outage indicator with emergency mode (red for emergency, yellow for normal outage)
void AlertLight_UI_Update_LightIndicator_Emergency(bool is_emergency) {
    // SAFETY: Don't access objects if not initialized
    if (objects.light_indicator == NULL) {
        return;
    }

    if (is_emergency) {
        lv_led_on(objects.light_indicator);
        lv_led_set_color(objects.light_indicator, lv_color_hex(0xff0000)); // Red for emergency
        lv_led_set_brightness(objects.light_indicator, 255);
    }
}

// Tick function - call this regularly
void AlertLight_UI_Tick(void) {
    ui_tick();

    // Automatic screen transition from init to boot after 5 seconds
    // This provides a fallback in case the main loop doesn't handle it
    if (init_screen_active) {
        // Check if 5 seconds have passed since init screen started
        if (millis() - init_screen_start_time >= 5000) {
            if (objects.boot != NULL) {
                printf("Auto-transitioning from init to boot screen (Tick)\n");

                // Load boot screen with animation
                lv_scr_load_anim(objects.boot, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
                init_screen_active = false;
                boot_screen_active = true;
                printf("Auto-transition to boot screen successful\n");
            } else {
                printf("ERROR: Cannot transition - boot screen is NULL\n");
            }
        }
    }

    // Handle WiFi indicator blinking
    // SAFETY: Only access wifi_indicator if it's been initialized (main screen created)
    if (wifi_blink_enabled && objects.wifi_indicator != NULL) {
        unsigned long now = millis();
        if (now - last_blink_time >= 500) {  // Blink every 500ms
            last_blink_time = now;
            blink_state = !blink_state;

            if (blink_state) {
                lv_led_on(objects.wifi_indicator);
                lv_led_set_color(objects.wifi_indicator, lv_color_hex(0xffa500)); // Orange
                lv_led_set_brightness(objects.wifi_indicator, 255);
            } else {
                lv_led_off(objects.wifi_indicator);
            }
        }
    }
}

// Boot screen functions
void AlertLight_UI_ShowBootScreen(void) {
    if (boot_screen_active) {
        return;  // Already showing
    }

    printf("Showing boot screen\n");
    boot_screen_active = false;  // Will be set to true after successful load

    // Use safe screen loading with comprehensive validation
    if (objects.boot == NULL) {
        printf("ERROR: Boot screen is NULL!\n");
        return;
    }

    // Load the boot screen
    lv_scr_load_anim(objects.boot, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
    boot_screen_active = true;
    printf("Boot screen loaded successfully\n");

    // Clear any existing text in the boot log textarea and set accumulated logs
    if (objects.boot_log != NULL) {
        lv_textarea_set_text(objects.boot_log, boot_log_text.c_str());
    }

    printf("Boot logs displayed\n");
}

void AlertLight_UI_AddBootLog(const char* message) {
    if (objects.boot_log == NULL) {
        // If boot log not available, just print to console
        printf("Boot: %s\n", message);
        return;
    }

    // Append message to boot log text
    boot_log_text += String(message) + "\n";

    // Keep only last 2048 characters to avoid memory issues (increased from 500)
    // Use smarter truncation that preserves complete messages
    if (boot_log_text.length() > 2048) {
        // Find the first newline after the truncation point to avoid cutting mid-message
        size_t truncate_from = boot_log_text.length() - 2048;
        int newline_pos = boot_log_text.indexOf('\n', truncate_from);

        if (newline_pos != -1 && newline_pos < static_cast<int>(boot_log_text.length())) {
            // Start from the character after the newline to keep messages complete
            boot_log_text = boot_log_text.substring(newline_pos + 1);
        } else {
            // Fallback: no newline found, just truncate
            boot_log_text = boot_log_text.substring(truncate_from);
        }
    }

    // Update the boot log textarea
    lv_textarea_set_text(objects.boot_log, boot_log_text.c_str());

    // Also print to console for debugging
    printf("Boot: %s\n", message);
}

void AlertLight_UI_HideBootScreen(void) {
    if (!boot_screen_active) {
        return;
    }

    if (objects.main == NULL) {
        printf("ERROR: Cannot hide boot screen - main screen is NULL\n");
        return;
    }

    printf("Hiding boot screen, switching to main screen\n");

    // Switch to main screen
    lv_scr_load_anim(objects.main, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
    boot_screen_active = false;
    main_screen_active = true;
    printf("Main screen loaded successfully\n");
}

bool AlertLight_UI_IsBootScreenActive(void) {
    return boot_screen_active;
}

// Enable/disable WiFi indicator blinking
void AlertLight_UI_Update_WiFi_Blink(bool blink) {
    wifi_blink_enabled = blink;
    if (!blink) {
        blink_state = false;
        last_blink_time = 0;
    }
}

// Get current WiFi blinking state
bool AlertLight_UI_IsWiFiBlinking(void) {
    return wifi_blink_enabled;
}

// Update clock display with current time (HH:MM:SS in 24h format)
void AlertLight_UI_Update_Clock(void) {
    if (!clock_label) {
        return;  // Clock not initialized
    }

    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    // Check if time is valid (not epoch 0)
    if (now < 1000000000) {
        lv_label_set_text(clock_label, "--:--:--");
        return;
    }

    // Format time as HH:MM:SS
    char time_str[9];
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d",
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    lv_label_set_text(clock_label, time_str);
}

#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects = {0};  // CRITICAL: Zero-initialize to prevent uninitialized pointers
lv_obj_t *tick_value_change_obj = NULL;
uint32_t active_theme_index = 0;

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 172, 320);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // top_section
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.top_section = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 172, 50);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xff333333), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // device_ip
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.device_ip = obj;
                    lv_obj_set_pos(obj, 8, 17);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff00aaff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "192.168.1.100");
                }
                {
                    // wifi_indicator
                    lv_obj_t *obj = lv_led_create(parent_obj);
                    objects.wifi_indicator = obj;
                    lv_obj_set_pos(obj, 144, 15);
                    lv_obj_set_size(obj, 20, 20);
                    lv_led_set_color(obj, lv_color_hex(0xff00ff00));
                    lv_led_set_brightness(obj, 255);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                }
                {
                    // device_port
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.device_port = obj;
                    lv_obj_set_pos(obj, 91, 17);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff00aaff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, ":8080");
                }
            }
        }
        {
            // alert_section
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.alert_section = obj;
            lv_obj_set_pos(obj, 0, 50);
            lv_obj_set_size(obj, 172, 135);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff0a0a0a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xff333333), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // alert_indicator
                    lv_obj_t *obj = lv_led_create(parent_obj);
                    objects.alert_indicator = obj;
                    lv_obj_set_pos(obj, 10, 14);
                    lv_obj_set_size(obj, 40, 40);
                    lv_led_set_color(obj, lv_color_hex(0xff00ff00));
                    lv_led_set_brightness(obj, 255);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                }
                {
                    // region_name
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.region_name = obj;
                    lv_obj_set_pos(obj, 60, 14);
                    lv_obj_set_size(obj, 111, LV_SIZE_CONTENT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Kyiv");
                }
                {
                    // alert_status
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.alert_status = obj;
                    lv_obj_set_pos(obj, 60, 39);
                    lv_obj_set_size(obj, 111, LV_SIZE_CONTENT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "No Alert");
                }
            }
        }
        {
            // light_section
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.light_section = obj;
            lv_obj_set_pos(obj, 0, 118);
            lv_obj_set_size(obj, 172, 202);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff0a0a0a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // light_title
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.light_title = obj;
                    lv_obj_set_pos(obj, 0, 8);
                    lv_obj_set_size(obj, 172, LV_SIZE_CONTENT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff888888), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "POWER OUTAGES");
                }
                {
                    // queue_container
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.queue_container = obj;
                    lv_obj_set_pos(obj, 12, 32);
                    lv_obj_set_size(obj, 148, 30);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // light_indicator
                            lv_obj_t *obj = lv_led_create(parent_obj);
                            objects.light_indicator = obj;
                            lv_obj_set_pos(obj, 123, 5);
                            lv_obj_set_size(obj, 20, 20);
                            lv_led_set_color(obj, lv_color_hex(0xfff5ff00));
                            lv_led_set_brightness(obj, 255);
                        }
                        {
                            // queue_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.queue_label = obj;
                            lv_obj_set_pos(obj, 6, 7);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xffaaaaaa), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Queue:");
                        }
                        {
                            // queue_value
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.queue_value = obj;
                            lv_obj_set_pos(obj, 60, 7);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffaa00), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "6.2");
                        }
                    }
                }
                {
                    // schedule_today
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.schedule_today = obj;
                    lv_obj_set_pos(obj, 12, 72);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffaaaaaa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Today:");
                }
                {
                    // outage_time_1
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.outage_time_1 = obj;
                    lv_obj_set_pos(obj, 12, 92);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffff6600), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "05:00-06:30");
                }
                {
                    // outage_time_2
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.outage_time_2 = obj;
                    lv_obj_set_pos(obj, 12, 109);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffff6600), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "13:30-17:30");
                }
            }
        }
    }
    
    tick_screen_main();
}

void delete_screen_main() {
    lv_obj_del(objects.main);
    objects.main = 0;
    objects.top_section = 0;
    objects.device_ip = 0;
    objects.wifi_indicator = 0;
    objects.device_port = 0;
    objects.alert_section = 0;
    objects.alert_indicator = 0;
    objects.region_name = 0;
    objects.alert_status = 0;
    objects.light_section = 0;
    objects.light_title = 0;
    objects.queue_container = 0;
    objects.light_indicator = 0;
    objects.queue_label = 0;
    objects.queue_value = 0;
    objects.schedule_today = 0;
    objects.outage_time_1 = 0;
    objects.outage_time_2 = 0;
}

void tick_screen_main() {
}

void create_screen_boot() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.boot = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 172, 320);
    {
        lv_obj_t *parent_obj = obj;
        {
            // top_section_1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.top_section_1 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 172, 50);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xff333333), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // boot_placeholder
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.boot_placeholder = obj;
                    lv_obj_set_pos(obj, 8, 17);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff00aaff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Booting up");
                }
                {
                    // wifi_indicator_1
                    lv_obj_t *obj = lv_led_create(parent_obj);
                    objects.wifi_indicator_1 = obj;
                    lv_obj_set_pos(obj, 144, 15);
                    lv_obj_set_size(obj, 20, 20);
                    lv_led_set_color(obj, lv_color_hex(0xff00ff00));
                    lv_led_set_brightness(obj, 255);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                }
            }
        }
        {
            // debug_section
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.debug_section = obj;
            lv_obj_set_pos(obj, 0, 50);
            lv_obj_set_size(obj, 172, 270);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // boot_log_header
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.boot_log_header = obj;
                    lv_obj_set_pos(obj, 32, -15);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Boot log");
                }
                {
                    // boot_log
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.boot_log = obj;
                    lv_obj_set_pos(obj, -10, 3);
                    lv_obj_set_size(obj, 160, 244);
                    lv_textarea_set_max_length(obj, 4096);  // Increased from 128 to 4096 for full boot logs
                    lv_textarea_set_text(obj, "debug log text very long string\nwith newlines long string");
                    lv_textarea_set_one_line(obj, false);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff0d8f0f), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    // Enable scrollbar for long boot logs
                    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_AUTO);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                }
            }
        }
    }
    
    tick_screen_boot();
}

void delete_screen_boot() {
    lv_obj_del(objects.boot);
    objects.boot = 0;
    objects.top_section_1 = 0;
    objects.boot_placeholder = 0;
    objects.wifi_indicator_1 = 0;
    objects.debug_section = 0;
    objects.boot_log_header = 0;
    objects.boot_log = 0;
}

void tick_screen_boot() {
}

void create_screen_init() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.init = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 172, 320);
    {
        lv_obj_t *parent_obj = obj;
        {
            // init_tab
            lv_obj_t *obj = lv_tabview_create(parent_obj, LV_DIR_TOP, 32);
            objects.init_tab = obj;
            lv_obj_set_pos(obj, -4, 0);
            lv_obj_set_size(obj, 180, 203);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_tabview_add_tab(parent_obj, "AlertLight");
                }
                {
                    lv_obj_t *obj = lv_qrcode_create(parent_obj, 170, lv_color_hex(0xff20429f), lv_color_hex(0xffe2f5fe));
                    lv_qrcode_update(obj, "https://github.com/coolerUA/AlertLight", 38);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
                }
            }
        }
    }
    
    tick_screen_init();
}

void delete_screen_init() {
    lv_obj_del(objects.init);
    objects.init = 0;
    objects.init_tab = 0;
}

void tick_screen_init() {
}



typedef void (*create_screen_func_t)();
create_screen_func_t create_screen_funcs[] = {
    create_screen_main,
    create_screen_boot,
    create_screen_init,
};
void create_screen(int screen_index) {
    create_screen_funcs[screen_index]();
}
void create_screen_by_id(enum ScreensEnum screenId) {
    create_screen_funcs[screenId - 1]();
}

typedef void (*delete_screen_func_t)();
delete_screen_func_t delete_screen_funcs[] = {
    delete_screen_main,
    delete_screen_boot,
    delete_screen_init,
};
void delete_screen(int screen_index) {
    delete_screen_funcs[screen_index]();
}
void delete_screen_by_id(enum ScreensEnum screenId) {
    delete_screen_funcs[screenId - 1]();
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_boot,
    tick_screen_init,
};
void tick_screen(int screen_index) {
    // Safety check: prevent out-of-bounds access and invalid indices
    if (screen_index >= 0 && screen_index < 3) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
    create_screen_boot();
    create_screen_init();
}

#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *boot;
    lv_obj_t *init;
    lv_obj_t *top_section;
    lv_obj_t *device_ip;
    lv_obj_t *wifi_indicator;
    lv_obj_t *device_port;
    lv_obj_t *alert_section;
    lv_obj_t *alert_indicator;
    lv_obj_t *region_name;
    lv_obj_t *alert_status;
    lv_obj_t *light_section;
    lv_obj_t *light_title;
    lv_obj_t *queue_container;
    lv_obj_t *light_indicator;
    lv_obj_t *queue_label;
    lv_obj_t *queue_value;
    lv_obj_t *schedule_today;
    lv_obj_t *outage_time_1;
    lv_obj_t *outage_time_2;
    lv_obj_t *top_section_1;
    lv_obj_t *boot_placeholder;
    lv_obj_t *wifi_indicator_1;
    lv_obj_t *debug_section;
    lv_obj_t *boot_log_header;
    lv_obj_t *boot_log;
    lv_obj_t *init_tab;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_BOOT = 2,
    SCREEN_ID_INIT = 3,
};

void create_screen_main();
void delete_screen_main();
void tick_screen_main();

void create_screen_boot();
void delete_screen_boot();
void tick_screen_boot();

void create_screen_init();
void delete_screen_init();
void tick_screen_init();

void create_screen_by_id(enum ScreensEnum screenId);
void delete_screen_by_id(enum ScreensEnum screenId);
void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/
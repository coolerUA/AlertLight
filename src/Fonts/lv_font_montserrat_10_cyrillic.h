#ifndef LV_FONT_MONTSERRAT_CYRILLIC_H
#define LV_FONT_MONTSERRAT_CYRILLIC_H

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_font_t lv_font_montserrat_10_cyrillic;
extern const lv_font_t lv_font_montserrat_12_cyrillic;
extern const lv_font_t lv_font_montserrat_14_cyrillic;
extern const lv_font_t lv_font_montserrat_16_cyrillic;
extern const lv_font_t lv_font_montserrat_18_cyrillic;
extern const lv_font_t lv_font_montserrat_20_cyrillic;
extern const lv_font_t lv_font_montserrat_24_cyrillic;

// Bold variants
extern const lv_font_t lv_font_montserrat_bold_10_cyrillic;
extern const lv_font_t lv_font_montserrat_bold_12_cyrillic;
extern const lv_font_t lv_font_montserrat_bold_14_cyrillic;
extern const lv_font_t lv_font_montserrat_bold_16_cyrillic;
extern const lv_font_t lv_font_montserrat_bold_18_cyrillic;
extern const lv_font_t lv_font_montserrat_bold_20_cyrillic;
extern const lv_font_t lv_font_montserrat_bold_24_cyrillic;

#ifdef __cplusplus
}
#endif

#endif // LV_FONT_MONTSERRAT_CYRILLIC_H

#include "LVGL_Diagnostics.h"

// Helper function to safely dereference pointers with NULL checks
static const char* safe_string(const char* str) {
    return str ? str : "<null>";
}

bool LVGL_DiagPrintDisplayInfo(void) {
    printf("\n========== LVGL Display Diagnostics ==========\n");

    // Get the default display
    lv_disp_t* disp = lv_disp_get_default();

    if (disp == NULL) {
        printf("ERROR: Default display is NULL!\n");
        printf("  This means lv_disp_drv_register() was never called or failed.\n");
        printf("  LVGL cannot function without a registered display driver.\n");
        printf("==============================================\n\n");
        return false;
    }

    printf("Default display found at: %p\n", disp);

    // Check display driver
    lv_disp_drv_t* driver = disp->driver;
    if (driver == NULL) {
        printf("ERROR: Display driver is NULL!\n");
        printf("  The display object exists but has no driver attached.\n");
        printf("  This is a critical internal LVGL error.\n");
        printf("==============================================\n\n");
        return false;
    }

    printf("Display driver at: %p\n", driver);
    printf("  Resolution: %d x %d\n", driver->hor_res, driver->ver_res);
    printf("  Flush callback: %p\n", (void*)driver->flush_cb);
    printf("  Full refresh: %d\n", driver->full_refresh);

    // Check draw buffer
    lv_disp_draw_buf_t* draw_buf = driver->draw_buf;
    if (draw_buf == NULL) {
        printf("WARNING: Draw buffer is NULL!\n");
    } else {
        printf("Draw buffer at: %p\n", draw_buf);
        printf("  buf1: %p\n", draw_buf->buf1);
        printf("  buf2: %p\n", draw_buf->buf2);
        printf("  buf_act: %p\n", draw_buf->buf_act);
        printf("  size: %lu\n", (unsigned long)draw_buf->size);
    }

    // Check active screens
    lv_obj_t* act_scr = lv_disp_get_scr_act(disp);
    printf("Active screen: %p\n", act_scr);

    if (act_scr != NULL) {
        // Note: LVGL 8.3.11 doesn't have class->name field
        printf("  Active screen class: %p\n", lv_obj_get_class(act_scr));
        printf("  Active screen display: %p\n", lv_obj_get_disp(act_scr));
    }

    // Check layer screens
    lv_obj_t* layer_top = lv_disp_get_layer_top(disp);
    lv_obj_t* layer_sys = lv_disp_get_layer_sys(disp);
    printf("Layer top: %p\n", layer_top);
    printf("Layer sys: %p\n", layer_sys);

    printf("Display diagnostics: PASS\n");
    printf("==============================================\n\n");
    return true;
}

bool LVGL_DiagPrintScreenInfo(lv_obj_t* screen, const char* screen_name) {
    printf("\n========== Screen Info: %s ==========\n", safe_string(screen_name));

    if (screen == NULL) {
        printf("ERROR: Screen object is NULL!\n");
        printf("  The screen was not created or has been deleted.\n");
        printf("==============================================\n\n");
        return false;
    }

    printf("Screen address: %p\n", screen);

    // Check object class
    const lv_obj_class_t* obj_class = lv_obj_get_class(screen);
    if (obj_class == NULL) {
        printf("ERROR: Object class is NULL! Object may be corrupted.\n");
        printf("==============================================\n\n");
        return false;
    }

    // Note: LVGL 8.3.11 doesn't have class->name field
    printf("Object class: %p\n", obj_class);

    // Check if this is actually a screen object
    lv_obj_t* parent = lv_obj_get_parent(screen);
    printf("Parent object: %p\n", parent);

    if (parent != NULL) {
        printf("WARNING: Screen has a parent! Screens should be top-level objects.\n");
        // Note: LVGL 8.3.11 doesn't have class->name field
        printf("  Parent class: %p\n", lv_obj_get_class(parent));
    }

    // Check display association
    lv_disp_t* obj_disp = lv_obj_get_disp(screen);
    printf("Associated display: %p\n", obj_disp);

    if (obj_disp == NULL) {
        printf("ERROR: Screen is not associated with any display!\n");
        printf("  Screens must be associated with a display to be loadable.\n");
        printf("==============================================\n\n");
        return false;
    }

    // Compare with default display
    lv_disp_t* default_disp = lv_disp_get_default();
    if (obj_disp != default_disp) {
        printf("WARNING: Screen is associated with a different display than the default!\n");
        printf("  Default display: %p\n", default_disp);
        printf("  Screen display: %p\n", obj_disp);
    }

    // Check child count
    uint32_t child_cnt = lv_obj_get_child_cnt(screen);
    printf("Child count: %lu\n", (unsigned long)child_cnt);

    // Check if screen is currently loaded
    lv_obj_t* act_scr = lv_scr_act();
    if (act_scr == screen) {
        printf("Status: CURRENTLY ACTIVE\n");
    } else {
        printf("Status: Inactive\n");
    }

    printf("Screen diagnostics: PASS\n");
    printf("==============================================\n\n");
    return true;
}

bool LVGL_DiagSafeLoadScreen(lv_obj_t* screen, const char* screen_name, bool use_animation) {
    printf("\n========== Safe Screen Load: %s ==========\n", safe_string(screen_name));

    // Step 1: Validate display system
    lv_disp_t* disp = lv_disp_get_default();
    if (disp == NULL) {
        printf("CRITICAL: Cannot load screen - no default display!\n");
        printf("  Call LVGL_DiagPrintDisplayInfo() for details.\n");
        printf("==============================================\n\n");
        return false;
    }

    printf("Display system OK: %p\n", disp);

    // Step 2: Validate screen object
    if (screen == NULL) {
        printf("ERROR: Screen object is NULL!\n");
        printf("  Cannot load a NULL screen.\n");
        printf("==============================================\n\n");
        return false;
    }

    printf("Screen object OK: %p\n", screen);

    // Step 3: Validate screen is associated with a display
    lv_disp_t* screen_disp = lv_obj_get_disp(screen);
    if (screen_disp == NULL) {
        printf("ERROR: Screen is not associated with any display!\n");
        printf("  This usually means the screen was created incorrectly.\n");
        printf("  Screens should be created with lv_obj_create(NULL) or lv_obj_create(lv_scr_act()).\n");
        printf("==============================================\n\n");
        return false;
    }

    printf("Screen display association OK: %p\n", screen_disp);

    // Step 4: Validate screen class
    const lv_obj_class_t* obj_class = lv_obj_get_class(screen);
    if (obj_class == NULL) {
        printf("ERROR: Screen has NULL class! Object is corrupted.\n");
        printf("==============================================\n\n");
        return false;
    }

    // Note: LVGL 8.3.11 doesn't have class->name field
    printf("Screen class OK: %p\n", obj_class);

    // Step 5: Check if already active
    lv_obj_t* current_screen = lv_scr_act();
    if (current_screen == screen) {
        printf("INFO: Screen is already active, skipping load.\n");
        printf("==============================================\n\n");
        return true;
    }

    printf("Current screen: %p (will be replaced)\n", current_screen);

    // Step 6: Attempt to load the screen
    printf("Attempting to load screen...\n");

    // Ensure LVGL is ready by running timer handler
    lv_timer_handler();

    if (use_animation) {
        printf("Using fade animation (200ms)\n");
        lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
    } else {
        printf("Loading immediately without animation\n");
        lv_scr_load(screen);
    }

    // Process any pending tasks
    lv_timer_handler();

    // Step 7: Verify the screen was loaded
    lv_obj_t* new_screen = lv_scr_act();
    if (new_screen != screen) {
        printf("WARNING: Screen load may have failed!\n");
        printf("  Expected active screen: %p\n", screen);
        printf("  Actual active screen: %p\n", new_screen);
        printf("  This might be normal if using animation (check after animation completes).\n");
    } else {
        printf("SUCCESS: Screen loaded successfully!\n");
    }

    printf("==============================================\n\n");
    return true;
}

bool LVGL_DiagCheckTimerSystem(void) {
    printf("\n========== LVGL Timer System Check ==========\n");

    // Get current tick
    uint32_t tick_now = lv_tick_get();
    printf("Current LVGL tick: %lu\n", (unsigned long)tick_now);

    if (tick_now == 0) {
        printf("WARNING: LVGL tick is 0!\n");
        printf("  The tick timer may not be running.\n");
        printf("  Verify that lv_tick_inc() is being called regularly.\n");
        printf("  Timer system: QUESTIONABLE\n");
        printf("==============================================\n\n");
        return false;
    }

    // Run timer handler and check if it processes anything
    printf("Running lv_timer_handler()...\n");
    uint32_t time_till_next = lv_timer_handler();
    printf("Time until next timer: %lu ms\n", (unsigned long)time_till_next);

    printf("Timer system: OPERATIONAL\n");
    printf("==============================================\n\n");
    return true;
}

bool LVGL_DiagFullSystemCheck(void) {
    printf("\n");
    printf("=====================================\n");
    printf("  LVGL FULL SYSTEM DIAGNOSTIC CHECK  \n");
    printf("=====================================\n\n");

    bool all_ok = true;

    // Check 1: Display system
    printf("Check 1/3: Display System\n");
    if (!LVGL_DiagPrintDisplayInfo()) {
        printf("FAILED: Display system has critical issues!\n");
        all_ok = false;
    } else {
        printf("PASSED: Display system initialized correctly.\n");
    }

    // Check 2: Timer system
    printf("\nCheck 2/3: Timer System\n");
    if (!LVGL_DiagCheckTimerSystem()) {
        printf("WARNING: Timer system may have issues!\n");
        // Don't fail the entire check for timer warnings
    } else {
        printf("PASSED: Timer system operational.\n");
    }

    // Check 3: Current screen
    printf("\nCheck 3/3: Active Screen\n");
    lv_obj_t* act_scr = lv_scr_act();
    if (!LVGL_DiagPrintScreenInfo(act_scr, "Active Screen")) {
        printf("FAILED: Active screen has issues!\n");
        all_ok = false;
    } else {
        printf("PASSED: Active screen valid.\n");
    }

    printf("\n=====================================\n");
    if (all_ok) {
        printf("  OVERALL RESULT: ALL CHECKS PASSED  \n");
        printf("  LVGL system is ready for operation  \n");
    } else {
        printf("  OVERALL RESULT: CRITICAL FAILURES   \n");
        printf("  LVGL may not function correctly!    \n");
        printf("  Review errors above for details.    \n");
    }
    printf("=====================================\n\n");

    return all_ok;
}

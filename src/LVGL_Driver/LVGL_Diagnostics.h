#pragma once

#include <lvgl.h>
#include <Arduino.h>

/**
 * @brief Comprehensive LVGL diagnostic utilities for ESP32
 *
 * This module provides detailed diagnostic information about LVGL's internal state,
 * particularly focused on display driver initialization and screen object validity.
 *
 * Use these functions to debug LVGL initialization issues and screen loading crashes.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Print comprehensive LVGL display diagnostics
 *
 * This function examines the LVGL display system and prints detailed information about:
 * - Default display object existence and validity
 * - Display driver configuration and callbacks
 * - Draw buffer configuration
 * - Active screen objects
 * - Screen hierarchy and parent relationships
 *
 * Call this function before attempting to load screens to verify proper initialization.
 *
 * @return true if display appears valid, false if critical issues detected
 */
bool LVGL_DiagPrintDisplayInfo(void);

/**
 * @brief Print detailed information about a specific screen object
 *
 * This function examines a screen object and prints:
 * - Object address and validity
 * - Object type and class
 * - Parent object and screen association
 * - Child object count
 * - Display association
 * - Object state flags
 *
 * @param screen Pointer to the screen object to examine
 * @param screen_name Human-readable name for logging (e.g., "Boot Screen")
 * @return true if screen appears valid, false if issues detected
 */
bool LVGL_DiagPrintScreenInfo(lv_obj_t* screen, const char* screen_name);

/**
 * @brief Safely attempt to load a screen with comprehensive error checking
 *
 * This function performs extensive validation before attempting to load a screen:
 * 1. Validates display system is initialized
 * 2. Validates screen object is non-NULL
 * 3. Validates screen is properly associated with a display
 * 4. Validates screen object type
 * 5. Attempts the screen load with error detection
 *
 * @param screen Screen object to load
 * @param screen_name Human-readable name for logging
 * @param use_animation If true, uses fade animation; if false, loads immediately
 * @return true if screen loaded successfully, false on error
 */
bool LVGL_DiagSafeLoadScreen(lv_obj_t* screen, const char* screen_name, bool use_animation);

/**
 * @brief Verify that the LVGL timer system is functioning
 *
 * This function checks that LVGL's timer system is operational and can process events.
 * It's critical that lv_timer_handler() is being called regularly.
 *
 * @return true if timer system appears functional, false otherwise
 */
bool LVGL_DiagCheckTimerSystem(void);

/**
 * @brief Complete system diagnostic - run all checks
 *
 * This function runs a comprehensive diagnostic suite including:
 * - Display system validation
 * - Timer system validation
 * - Active screen validation
 * - Memory allocation checks
 *
 * Call this function in setup() after LVGL initialization to verify everything is correct.
 *
 * @return true if all systems operational, false if any issues detected
 */
bool LVGL_DiagFullSystemCheck(void);

#ifdef __cplusplus
}
#endif

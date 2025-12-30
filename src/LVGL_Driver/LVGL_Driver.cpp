/*****************************************************************************
  | File        :   LVGL_Driver.c
  
  | help        : 
    The provided LVGL library file must be installed first
******************************************************************************/
#include "LVGL_Driver.h"

// Display driver structures - MUST be static with proper lifetime management
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ LVGL_BUF_LEN ];
static lv_color_t buf2[ LVGL_BUF_LEN ];
static lv_disp_drv_t disp_drv;        // Display driver - must persist for LVGL lifetime
static lv_indev_drv_t indev_drv;      // Input device driver - must persist for LVGL lifetime

// Alternative: Allocate buffers in SPIRAM for larger displays
// static lv_color_t* buf1 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN, MALLOC_CAP_SPIRAM);
// static lv_color_t* buf2 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN, MALLOC_CAP_SPIRAM);
    


/* Serial debugging */
void Lvgl_print(const char * buf)
{
    // Serial.printf(buf);
    // Serial.flush();
}

/*  Display flushing 
    Displays LVGL content on the LCD
    This function implements associating LVGL data to the LCD screen
*/
void Lvgl_Display_LCD( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
  LCD_addWindow(area->x1, area->y1, area->x2, area->y2, ( uint16_t *)&color_p->full);
  lv_disp_flush_ready( disp_drv );
}
/*Read the touchpad*/
void Lvgl_Touchpad_Read( lv_indev_drv_t * indev_drv, lv_indev_data_t * data )
{
  // NULL
}
void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}
void Lvgl_Init(void)
{
  // Initialize LVGL core
  lv_init();

  // Initialize draw buffers
  lv_disp_draw_buf_init( &draw_buf, buf1, buf2, LVGL_BUF_LEN);

  // Initialize the display driver
  // CRITICAL: disp_drv is now a file-scope static variable (declared at top of file)
  // This ensures it persists for the entire lifetime of the program
  // LVGL stores pointers to this structure, so it must never go out of scope
  lv_disp_drv_init( &disp_drv );

  // Configure display driver
  disp_drv.hor_res = LVGL_WIDTH;
  disp_drv.ver_res = LVGL_HEIGHT;
  disp_drv.flush_cb = Lvgl_Display_LCD;
  disp_drv.full_refresh = 1;                    // Always redraw the whole screen
  disp_drv.draw_buf = &draw_buf;

  // Register the display driver with LVGL
  // This returns a pointer to the internal display object (lv_disp_t*)
  // LVGL manages this object internally
  lv_disp_t* disp = lv_disp_drv_register( &disp_drv );

  if (disp == NULL) {
    printf("CRITICAL ERROR: Failed to register display driver!\n");
    printf("LVGL cannot function without a display.\n");
    return;
  }

  printf("Display driver registered successfully at %p\n", disp);

  // Initialize the input device driver
  // CRITICAL: indev_drv is now a file-scope static variable
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = Lvgl_Touchpad_Read;
  lv_indev_drv_register( &indev_drv );

  // Note: Default screen is automatically created by lv_init()
  // We don't need to create it manually

  // Start the LVGL tick timer
  const esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &example_increase_lvgl_tick,
    .name = "lvgl_tick"
  };
  esp_timer_handle_t lvgl_tick_timer = NULL;
  esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
  esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);

  printf("LVGL initialization complete\n");
}
void Timer_Loop(void)
{
  lv_timer_handler(); /* let the GUI do its work */
  // delay( 5 );
}
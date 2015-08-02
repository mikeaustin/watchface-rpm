//
// main.c
//
// Copyright 2015 Mike Austin | See LICENSE.md for details
//

#include <pebble.h>
  
Window *main_window;
Layer  *gauge_layer, *dial_layer;

GBitmap *numbers_bitmap, *background_bitmap;
GBitmap *lcd_font_bitmap, *lcd_font_digits[10];
GBitmap *mph_text_bitmap, *kph_text_bitmap;
GBitmap *lights_bitmap, *lights_icons[2];

int   charge_percent = 0;
bool  bluetooth_connected = false;
float angle_advance = 5;
bool  angle_forward = false;
bool  background_captured = false;

void gauge_layer_update(Layer *layer, GContext *ctx);
void dial_layer_update(Layer *layer, GContext *ctx);
void dial_animation_callback(void *data);

static void main_window_load(Window *window)
{
    // Create background bitmap

    background_bitmap = gbitmap_create_blank(GSize(144, 168), GBitmapFormat8Bit);

    // Create numbers bitmaps

    numbers_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NUMBERS);

    // Create MPH text bitmaps

    mph_text_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MPH_TEXT);
    kph_text_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_KPH_TEXT);

    // Create LCD bitmaps

    lcd_font_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LCD_FONT);

    for (int i = 0; i < 10; i++)
    {
        lcd_font_digits[i] = gbitmap_create_as_sub_bitmap(lcd_font_bitmap, GRect(i * 15, 0, 15, 25));
    }

    // Create lights bitmaps

    lights_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GAUGE_LIGHTS);

    for (int i = 0; i < 2; i++)
    {
        lights_icons[i] = gbitmap_create_as_sub_bitmap(lights_bitmap, GRect(i * 22, 0, 22, 15));
    }

    // Create gauge layer

    gauge_layer = layer_create(GRect(0, 0, 144, 168));

    layer_set_update_proc(gauge_layer, gauge_layer_update);
    layer_add_child(window_get_root_layer(window), gauge_layer);

    // create dial layer

    dial_layer = layer_create(GRect(0, 0, 144, 168));

    layer_set_update_proc(dial_layer, dial_layer_update);
    layer_add_child(window_get_root_layer(window), dial_layer);
}
 
static void main_window_unload(Window *window)
{
    // Destroy layers

    layer_destroy(dial_layer);
    layer_destroy(gauge_layer);

    // Destroy bitmaps

    for (int i = 0; i < 2; i++)
    {
        gbitmap_destroy(lights_icons[i]);
    }

    gbitmap_destroy(lights_bitmap);

    for (int i = 0; i < 10; i++)
    {
        gbitmap_destroy(lcd_font_digits[i]);
    }

    gbitmap_destroy(lcd_font_bitmap);

    gbitmap_destroy(kph_text_bitmap);
    gbitmap_destroy(mph_text_bitmap);

    gbitmap_destroy(numbers_bitmap);
    gbitmap_destroy(background_bitmap);
}

static void tick_timer_handler(struct tm *tick_time, TimeUnits units_changed)
{
    if (charge_percent >= 20)
    {
        angle_forward = true;

        app_timer_register(50, dial_animation_callback, (void *)50);
    }
    else layer_mark_dirty(dial_layer);
}

static void battery_state_handler(BatteryChargeState charge)
{
    charge_percent = charge.charge_percent;
    //charge_percent = 10;

    background_captured = false;
    layer_mark_dirty(gauge_layer);
}

static void bluetooth_connection_handler(bool connected)
{
    bluetooth_connected = connected;
    //bluetooth_connected = false;

    background_captured = false;
    layer_mark_dirty(gauge_layer);
}

static void init()
{
    main_window = window_create();
 
    window_set_window_handlers(main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    window_set_background_color(main_window, GColorBlack);
 
    window_stack_push(main_window, true);

    tick_timer_service_subscribe(MINUTE_UNIT, tick_timer_handler);

    battery_state_service_subscribe(battery_state_handler);
    battery_state_handler(battery_state_service_peek());

    bluetooth_connection_handler(bluetooth_connection_service_peek());
    bluetooth_connection_service_subscribe(bluetooth_connection_handler);
}
 
static void deinit()
{
    bluetooth_connection_service_unsubscribe();
    battery_state_service_unsubscribe();
    tick_timer_service_unsubscribe();

    window_destroy(main_window);
}
 
int main(void)
{
    init();

    app_event_loop();

    deinit();
}

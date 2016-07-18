//
// main.c
//
// Copyright 2015 Mike Austin | See LICENSE.md for details
//

#include <pebble.h>

Layer *window_layer;
GRect window_bounds;

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
bool  show_two_hands = true;

void gauge_layer_update(Layer *layer, GContext *ctx);
void dial_layer_update(Layer *layer, GContext *ctx);
void dial_animation_callback(void *data);

static void main_window_load(Window *window)
{
    window_layer = window_get_root_layer(window);
    window_bounds = layer_get_bounds(window_layer);

    // Create background bitmap

    background_bitmap = gbitmap_create_blank(window_bounds.size, GBitmapFormat8Bit);

    // Create numbers bitmaps

    if (show_two_hands)
        numbers_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NUMBERS_CLOCK);
    else
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

    gauge_layer = layer_create(window_bounds);

    layer_set_update_proc(gauge_layer, gauge_layer_update);
    layer_add_child(window_get_root_layer(window), gauge_layer);

    // create dial layer

    dial_layer = layer_create(window_bounds);

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
    //if (false)
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

static void inbox_received_handler(DictionaryIterator *iter, void *context)
{
    Tuple *two_hands_t = dict_find(iter, MESSAGE_KEY_TwoHands);
    if (two_hands_t && show_two_hands != (two_hands_t->value->int32 == 1))
    {
        show_two_hands = two_hands_t->value->int32 == 1;

        persist_write_data(0, &show_two_hands, sizeof(show_two_hands));

        gbitmap_destroy(numbers_bitmap);

        if (show_two_hands)
            numbers_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NUMBERS_CLOCK);
        else
            numbers_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NUMBERS);

        background_captured = false;

        layer_mark_dirty(gauge_layer);
        layer_mark_dirty(dial_layer);
    }
}

static void init()
{
    persist_read_data(0, &show_two_hands, sizeof(show_two_hands));

    app_message_register_inbox_received(inbox_received_handler);
    app_message_open(128, 128);

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

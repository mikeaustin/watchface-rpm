//
// dial_layer.c
//
// Copyright 2015 Mike Austin | See LICENSE.md for details
//

#include <pebble.h>

extern Layer *window_layer;
extern GRect window_bounds;

extern Layer  *gauge_layer, *dial_layer;

extern GBitmap *numbers_bitmap, *background_bitmap;
extern GBitmap *lcd_font_bitmap, *lcd_font_digits[10];
extern GBitmap *mph_text_bitmap, *kph_text_bitmap;
extern GBitmap *lights_bitmap, *lights_icons[1];

extern int   charge_percent;
extern bool  bluetooth_connected;
extern float angle_advance;
extern bool  angle_forward;
extern bool  background_captured;
extern bool  show_two_hands;

GPoint point_for_angle_radius(int angle, int radius);
void graphics_copy_frame_buffer_to_bitmap(GContext *ctx, GBitmap *bitmap);

// Layer update function that draws gauge dial and LCD text

void dial_layer_update(Layer *layer, GContext *ctx)
{
    GRect frame = layer_get_frame(layer);
    GPoint center = GPoint(frame.size.w / 2, frame.size.h / 2 - 1);

    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);

    int hour  = tick_time->tm_hour % (clock_is_24h_style() ? 24 : 12),
        min   = tick_time->tm_min,
        month = tick_time->tm_mon + 1,
        day   = tick_time->tm_mday;

    if (!clock_is_24h_style() && hour == 0) hour = 12; 

    // Draw Hour LCD

    static char time_buffer[] = "240";

    if (show_two_hands)
        snprintf(time_buffer, sizeof(time_buffer), "%d", month % 10 * 100 + day);
    else
        snprintf(time_buffer, sizeof(time_buffer), "%d", hour * 10 + min / 6);

    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    int x = window_bounds.size.w / 2 - strlen(time_buffer) * 15 + 22;
    for (unsigned i = 0; i < strlen(time_buffer); i++)
    {
        graphics_draw_bitmap_in_rect(ctx, lcd_font_digits[time_buffer[i] - '0'], GRect(x + i * 15, (window_bounds.size.h - 27) / 2 + 45, 15, 27));
    }

    // Draw dials

    int angles[] = { hour * (360 / 12.0) + min * (360 / (60 * 12.0)),
                     (show_two_hands ? min * (360 / 60.0) : min * (180 / 60.0) - 120) + angle_advance - 5 };
    int sizes[]  = { 36, 63 };

    for (int i = (show_two_hands ? 0 : 1); i < 2; i++)
    {
        // Draw dial stroke

        GPoint p1 = point_for_angle_radius(TRIG_MAX_ANGLE * ((angles[i] - 90) / 360.0), 2);
        GPoint p2 = point_for_angle_radius(TRIG_MAX_ANGLE * (angles[i] / 360.0), sizes[i]);

        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_stroke_width(ctx, 4);

        graphics_draw_line(ctx, GPoint(p1.x + center.x, p1.y + center.y), GPoint(p2.x + center.x, p2.y + center.y));
        graphics_draw_line(ctx, GPoint(-p1.x + center.x, -p1.y + center.y), GPoint(p2.x + center.x, p2.y + center.y));

        // Draw dial

        graphics_context_set_stroke_color(ctx, GColorOrange);
        graphics_context_set_stroke_width(ctx, 2);

        graphics_draw_line(ctx, GPoint(p1.x + center.x, p1.y + center.y), GPoint(p2.x + center.x, p2.y + center.y));
        graphics_draw_line(ctx, GPoint(-p1.x + center.x, -p1.y + center.y), GPoint(p2.x + center.x, p2.y + center.y));
    }

    // Draw dial center cap

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, GPoint(center.x, center.y), 10);

    // Draw black line to center gauge

    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, GPoint(0, 0), GPoint(0, 168));
}

void dial_animation_callback(void *data) 
{
    if (angle_forward)
    {
        angle_advance *= 1.5;

        if (angle_advance > 35)
        {
            angle_advance = 35;
            angle_forward = false;
        }

        app_timer_register((int)data, dial_animation_callback, data); 
    }
    else
    {
        angle_advance *= 0.9;

        if (angle_advance > 5)
        {
            app_timer_register((int)data, dial_animation_callback, data); 
        }
        else angle_advance = 5;
    }

    layer_mark_dirty(dial_layer);
}

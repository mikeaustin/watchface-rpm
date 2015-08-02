//
// gauge_layer.c
//
// Copyright 2015 Mike Austin | See LICENSE.md for details
//

#include <pebble.h>

extern Layer  *gauge_layer, *dial_layer;

extern GBitmap *numbers_bitmap, *background_bitmap;
extern GBitmap *lcd_font_bitmap, *lcd_font_digits[10];
extern GBitmap *mph_text_bitmap, *kph_text_bitmap;
extern GBitmap *lights_bitmap, *lights_icons[2];

extern int   charge_percent;
extern bool  bluetooth_connected;
extern float angle_advance;
extern bool  angle_forward;
extern bool  background_captured;

GPoint point_for_angle_radius(int angle, int radius);
void graphics_copy_frame_buffer_to_bitmap(GContext *ctx, GBitmap *bitmap);

// Layer update function that draws gauge background, frame, and ticks - also draws LCD background

void gauge_layer_update(Layer *layer, GContext *ctx)
{
    if (background_captured)
    {
        graphics_draw_bitmap_in_rect(ctx, background_bitmap, GRect(0, 0, 144, 168));

        return;
    }

    // Draw texture

    int size = 4;
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
    for (int x = -144; x < 144 / size; x++)
    {
        graphics_draw_line(ctx, GPoint(x * size, 0), GPoint(x * size + 168, 168));
    }

    GRect frame = layer_get_frame(layer);
    GPoint center = GPoint(frame.size.w / 2, frame.size.h / 2 - 1);

    // Draw outer frame

    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_circle(ctx, GPoint(center.x, center.y), 81);

    graphics_context_set_stroke_color(ctx, GColorLightGray);
    graphics_context_set_stroke_width(ctx, 6);
    graphics_draw_circle(ctx, GPoint(center.x, center.y), 78);

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_circle(ctx, GPoint(center.x, center.y), 74);

    graphics_context_set_stroke_color(ctx, GColorDarkGray);
    graphics_context_set_stroke_width(ctx, 4);
    graphics_draw_circle(ctx, GPoint(center.x, center.y), 72);

    // Draw face

    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_circle(ctx, GPoint(center.x, center.y), 68);

    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, GPoint(center.x, center.y + 0), 64);

    // Draw ticks

    for (int i = -120; i <= 120; i += 1)
    {
        int angle = TRIG_MAX_ANGLE * (i / 360.0);

        graphics_context_set_stroke_color(ctx, GColorBlack);

        if (i >= 80)
        {
            graphics_context_set_stroke_color(ctx, GColorRed);
        }

        if (i % (240 / 8) == 0)
        {
            // Draw major ticks

            graphics_context_set_stroke_width(ctx, 2);

            GPoint p1 = point_for_angle_radius(angle, 58), p2 = point_for_angle_radius(angle, 68);
            graphics_draw_line(ctx, GPoint(p1.x + center.x, p1.y + center.y), GPoint(p2.x + center.x, p2.y + center.y));
        }
        else if (i % (240 / 16) == 0)
        {
            // Draw half major ticks

            graphics_context_set_stroke_width(ctx, 2);

            GPoint p1 = point_for_angle_radius(angle, 60), p2 = point_for_angle_radius(angle, 68);
            graphics_draw_line(ctx, GPoint(p1.x + center.x, p1.y + center.y), GPoint(p2.x + center.x, p2.y + center.y));
        }
        else if (i % (240 / 48) == 0)
        {
            // Draw minor ticks

            graphics_context_set_stroke_width(ctx, 1);

            GPoint p1 = point_for_angle_radius(angle, 63), p2 = point_for_angle_radius(angle, 65);
            graphics_draw_line(ctx, GPoint(p1.x + center.x, p1.y + center.y), GPoint(p2.x + center.x, p2.y + center.y));
        }
    }

    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 2);

    graphics_draw_circle(ctx, GPoint(center.x, center.y), 68);

    // Draw gauge numbers

    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, numbers_bitmap, GRect(0, 0, 144, 168));

    graphics_draw_bitmap_in_rect(ctx, clock_is_24h_style() ? kph_text_bitmap : mph_text_bitmap, GRect((144 - 30) / 2, 97, 30, 20));

    // Draw MPH Background

    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_rect(ctx, GRect((144 - 52) / 2, 115, 52, 25), 3, GCornersAll);

    graphics_context_set_fill_color(ctx, GColorPictonBlue);
    //graphics_context_set_fill_color(ctx, GColorMintGreen);
    graphics_fill_rect(ctx, GRect((144 - 50) / 2, 116, 50, 23), 3, GCornersAll);

    // Draw gauge icons

    if (charge_percent < 20)
    {
        graphics_draw_bitmap_in_rect(ctx, lights_icons[0], GRect(3, 2, 22, 15));
    }

    if (!bluetooth_connected)
    {
        graphics_draw_bitmap_in_rect(ctx, lights_icons[1], GRect(144 - 22 - 3, 2, 22, 15));
    }

    graphics_copy_frame_buffer_to_bitmap(ctx, background_bitmap);

    background_captured = true;
}

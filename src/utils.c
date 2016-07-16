//
// utils.c
//
// Copyright 2015 Mike Austin | See LICENSE.md for details
//

#include <pebble.h>

// Helper function to calculate dial angle

GPoint point_for_angle_radius(int angle, int radius)
{
    return (GPoint) { sin_lookup(angle) * radius / TRIG_MAX_RATIO, -cos_lookup(angle) * radius / TRIG_MAX_RATIO };
}

// Helper function to copy frame buffer to bitmap

void graphics_copy_frame_buffer_to_bitmap(GContext *ctx, GBitmap *bitmap)
{
    GBitmap *buffer = graphics_capture_frame_buffer(ctx);
    uint8_t *data = gbitmap_get_data(buffer);

    GRect bounds = gbitmap_get_bounds(bitmap);
    uint8_t *data2 = gbitmap_get_data(bitmap);

    for (int y = 0; y < bounds.size.h; y++)
    {
        GBitmapDataRowInfo info = gbitmap_get_data_row_info(buffer, y);
        GBitmapDataRowInfo info2 = gbitmap_get_data_row_info(bitmap, y);

        for(int x = info.min_x; x <= info.max_x; x++)
        //for (int x = 0; i < bounds.size.w * bounds.size.h; i++)
        {
            memset(&info2.data[x], info.data[x], 1);
            //data2[x] = info.data[x];
            //data2[i] = data[i];
        }
    }

    graphics_release_frame_buffer(ctx, buffer);
}

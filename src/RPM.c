#include <pebble.h>
  
static Window *main_window;
static Layer  *gauge_layer, *dial_layer;

static GBitmap *numbers_bitmap;
static GBitmap *lcd_font_bitmap, *lcd_font_digits[10];
static GBitmap *mph_text_bitmap, *kph_text_bitmap;

// Helper function to calculate dial angle

GPoint pointForAngle(int angle, int radius)
{
    return (GPoint) { sin_lookup(angle) * radius / TRIG_MAX_RATIO, -cos_lookup(angle) * radius / TRIG_MAX_RATIO };
}

// Layer update function that draws gauge background, frame, and ticks - also draws LCD background

void gauge_layer_update(Layer *layer, GContext *ctx)
{
    // Draw background

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
            graphics_context_set_stroke_width(ctx, 2);
        }

        if (i % (240 / 8) == 0)
        {
            // Draw major ticks

            graphics_context_set_stroke_width(ctx, 2);

            GPoint p = pointForAngle(angle, 58), p2 = pointForAngle(angle, 68);
            graphics_draw_line(ctx, GPoint(p.x + center.x, p.y + center.y), GPoint(p2.x + center.x, p2.y + center.y));
        }
        else if (i % (240 / 16) == 0)
        {
            // Draw half major ticks

            graphics_context_set_stroke_width(ctx, 2);

            GPoint point2 = (GPoint) { sin_lookup(angle) * 60 / TRIG_MAX_RATIO + 144 / 2, -cos_lookup(angle) * 60 / TRIG_MAX_RATIO + center.y };
            GPoint point = (GPoint) { sin_lookup(angle) * 68 / TRIG_MAX_RATIO + 144 / 2, -cos_lookup(angle) * 68 / TRIG_MAX_RATIO + center.y };

            graphics_draw_line(ctx, point2, point);
        }
        else if (i % (240 / 48) == 0)
        {
            // Draw minor ticks

            graphics_context_set_stroke_width(ctx, 1);

            GPoint point2 = (GPoint) { sin_lookup(angle) * 63 / TRIG_MAX_RATIO + 144 / 2, -cos_lookup(angle) * 63 / TRIG_MAX_RATIO + center.y };
            GPoint point = (GPoint) { sin_lookup(angle) * 65 / TRIG_MAX_RATIO + 144 / 2, -cos_lookup(angle) * 65 / TRIG_MAX_RATIO + center.y };

            graphics_draw_line(ctx, point2, point);
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
}

// Layer update function that draws gauge dial and LCD text

void dial_layer_update(Layer *layer, GContext *ctx)
{
    GRect frame = layer_get_frame(layer);
    GPoint center = GPoint(frame.size.w / 2, frame.size.h / 2 - 1);

    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);

    int hour = tick_time->tm_hour % (clock_is_24h_style() ? 24 : 12),
        min  = tick_time->tm_min;

    int angle = min * (180 / 60.0) - 120;

    // Draw dial stroke

    GPoint pp = pointForAngle(TRIG_MAX_ANGLE * ((angle - 90) / 360.0), 2);
    GPoint p = pointForAngle(TRIG_MAX_ANGLE * (angle / 360.0), 63);

    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 4);

    graphics_draw_line(ctx, GPoint(pp.x + center.x, pp.y + center.y), GPoint(p.x + center.x, p.y + center.y));
    graphics_draw_line(ctx, GPoint(-pp.x + center.x, -pp.y + center.y), GPoint(p.x + center.x, p.y + center.y));

    // Draw dial

    graphics_context_set_stroke_color(ctx, GColorOrange);
    graphics_context_set_stroke_width(ctx, 2);

    graphics_draw_line(ctx, GPoint(pp.x + center.x, pp.y + center.y), GPoint(p.x + center.x, p.y + center.y));
    graphics_draw_line(ctx, GPoint(-pp.x + center.x, -pp.y + center.y), GPoint(p.x + center.x, p.y + center.y));

    // Draw dial center cap

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, GPoint(center.x, center.y), 10);

    // Draw Hour LCD

    static char time_buffer[] = "240";

    snprintf(time_buffer, sizeof(time_buffer), "%d", hour * 10 + min / 6);

    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    int x = 94 - strlen(time_buffer) * 15;
    for (unsigned i = 0; i < strlen(time_buffer); i++)
    {
        graphics_draw_bitmap_in_rect(ctx, lcd_font_digits[time_buffer[i] - '0'], GRect(x + i * 15, 115, 15, 27));
    }

    // Draw black line to center gauge

    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, GPoint(0, 0), GPoint(0, 168));
}

static void main_window_load(Window *window)
{
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

    for (int i = 0; i < 10; i++)
    {
        gbitmap_destroy(lcd_font_digits[i]);
    }

    gbitmap_destroy(lcd_font_bitmap);

    gbitmap_destroy(kph_text_bitmap);
    gbitmap_destroy(mph_text_bitmap);
    gbitmap_destroy(numbers_bitmap);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    layer_mark_dirty(dial_layer);
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

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}
 
static void deinit()
{
    tick_timer_service_unsubscribe();

    window_destroy(main_window);
}
 
int main(void)
{
    init();

    app_event_loop();

    deinit();
}

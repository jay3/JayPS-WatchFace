#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screen_speed.h"
#include "screens.h"
#include "screen_config.h"

#define NUMBER_OF_IMAGES 13
#define TOTAL_IMAGE_SLOTS 4
#define NOT_USED -1

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_NUM_0, RESOURCE_ID_IMAGE_NUM_1, RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3, RESOURCE_ID_IMAGE_NUM_4, RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6, RESOURCE_ID_IMAGE_NUM_7, RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9,RESOURCE_ID_IMAGE_NUM_DOT,RESOURCE_ID_IMAGE_NUM_COLON,RESOURCE_ID_IMAGE_NUM_MINUS
};
enum {
  IMAGE_NUM_DOT = 10,
  IMAGE_NUM_COLON,
  IMAGE_NUM_MINUS
};

GBitmap *images[TOTAL_IMAGE_SLOTS];
BitmapLayer *image_layers[TOTAL_IMAGE_SLOTS];
int image_slots[TOTAL_IMAGE_SLOTS] = {NOT_USED,NOT_USED,NOT_USED,NOT_USED};

//TextLayer *distance_layer;
//TextLayer *avg_layer;
Layer *line_layer;
Layer *bottom_layer;

#if ROTATION
enum {
  ROTATION_DISABLED,
  ROTATION_MIN,
  ROTATION_SPEED = ROTATION_MIN,
  ROTATION_HEARTRATE,
  ROTATION_ALTITUDE,
  ROTATION_MAX,
};
int rotation = ROTATION_DISABLED;
static AppTimer *rotation_timer;
#endif

void speed_layer_update_proc(Layer *layer, GContext* ctx) {
  SpeedLayer *speed_layer = &s_data.screenA_layer.speed_layer;

  graphics_context_set_fill_color(ctx, BG_COLOR_SPEED_DATA);
  GRect bounds = layer_get_frame(speed_layer->layer);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  if (speed_layer->text && strlen(speed_layer->text) > 0) {
    
    // TODO: check if it's the same text?
    
    int len = strlen(speed_layer->text);

    if (len > 4) {
      len = 4;
    }

    // number of dots
    int dots = 0;
    for (int c=0; c < len; c++) {
      if (speed_layer->text[c] == ',') {
        // convert commas to dots (older app sent localized numbers...)
        speed_layer->text[c] = '.';
      }
      if (speed_layer->text[c] == '.' || speed_layer->text[c] == ':' || speed_layer->text[c] == '-') {
        // dot, colon or minus, same width
        dots++;
      }
    }

    // get the size
    int size = (len - dots) * CHAR_WIDTH + dots * DOT_WIDTH;

    int leftpos = PAGE_OFFSET_X + (PAGE_W - size) / 2;

    // clean up old layers
    for(int n=0; n < TOTAL_IMAGE_SLOTS; n++) {
      if(image_slots[n] != NOT_USED) {
        layer_remove_from_parent(bitmap_layer_get_layer(image_layers[n]));
        bitmap_layer_destroy(image_layers[n]);
        gbitmap_destroy(images[n]);
        image_slots[n] = NOT_USED;
      }
    }

    for(int c=0; c < len; c++) {

      int digit_value = -1;
      if (speed_layer->text[c] == '.') {
        digit_value = IMAGE_NUM_DOT;
      } else if (speed_layer->text[c] == ':') {
        digit_value = IMAGE_NUM_COLON;
      } else if (speed_layer->text[c] == '-') {
        digit_value = IMAGE_NUM_MINUS;
      } else {
        digit_value = speed_layer->text[c] - '0';
      }
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "%d=%c digit_value=%d", c, speed_layer->text[c], digit_value);

      if (digit_value >= 0 && digit_value < NUMBER_OF_IMAGES) {
        images[c] = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[digit_value]);
        image_layers[c] = bitmap_layer_create(gbitmap_get_bounds(images[c]));
        bitmap_layer_set_bitmap(image_layers[c], images[c]);
        
        GRect frame = layer_get_frame(bitmap_layer_get_layer(image_layers[c]));
        frame.origin.x = leftpos;
        frame.origin.y = 10;
        layer_set_frame(bitmap_layer_get_layer(image_layers[c]), frame);

        layer_add_child(s_data.screenA_layer.speed_layer.layer, bitmap_layer_get_layer(image_layers[c]));
        image_slots[c] = digit_value;
      }

      if (digit_value == IMAGE_NUM_DOT || digit_value == IMAGE_NUM_COLON || digit_value == IMAGE_NUM_MINUS) {
        // dot or colon, same width
        leftpos += DOT_WIDTH;
      } else {
        leftpos += CHAR_WIDTH;
      }
    }
  }
}
void speed_layer_init(SpeedLayer *speed_layer, GRect frame) {
  speed_layer->layer = layer_create(frame);
  layer_set_update_proc(speed_layer->layer, speed_layer_update_proc);
}
void speed_layer_set_text(SpeedLayer *speed_layer, char *textdata) {
  speed_layer->text = textdata;
}
void page_speed_update_proc(Layer *page_speed, GContext* ctx) {
  //vibes_short_pulse();
}
#ifdef PBL_PLATFORM_CHALK
  #define PAGE_SPEED_TOP_H SCREEN_H / 2 - TOPBAR_HEIGHT + 10
#else
  #define PAGE_SPEED_TOP_H SCREEN_H / 2 - TOPBAR_HEIGHT + 20
#endif

void line_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;
  graphics_context_set_stroke_color(ctx, COLOR_LINES);
  graphics_draw_line(ctx, GPoint(PAGE_OFFSET_X + PAGE_W / 2, PAGE_SPEED_TOP_H + 2), GPoint(PAGE_OFFSET_X + PAGE_W / 2, PAGE_H - 2));
}
void screen_speed_layer_init(Window* window) {
  s_data.screenA_layer.field_top.type = config.screenA_top_type;
  s_data.screenA_layer.field_bottom_left.type = config.screenA_bottom_left_type;
  s_data.screenA_layer.field_bottom_right.type = config.screenA_bottom_right_type;

  s_data.page_speed = layer_create(PAGE_GRECT);
  layer_set_update_proc(s_data.page_speed, page_speed_update_proc);
  layer_add_child(window_get_root_layer(window), s_data.page_speed);

  //speed_layer_init(&s_data.screenA_layer.speed_layer,GRect(0,0,PAGE_W,84));
  speed_layer_init(&s_data.screenA_layer.speed_layer, GRect(0, 0, SCREEN_W, PAGE_SPEED_TOP_H));
  speed_layer_set_text(&s_data.screenA_layer.speed_layer, s_data.speed);
  layer_add_child(s_data.page_speed, s_data.screenA_layer.speed_layer.layer);

  Layer *window_layer = window_get_root_layer(window);
  line_layer = layer_create(layer_get_frame(window_layer));
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(s_data.page_speed, line_layer);

  // BEGIN top "speed"
  // s_data.screenA_layer.field_top.title_layer NOT used

  s_data.screenA_layer.field_top.unit_layer = text_layer_create(GRect(PAGE_OFFSET_X, PAGE_SPEED_TOP_H - 24, PAGE_W, 22));
  set_layer_attr_full(s_data.screenA_layer.field_top.unit_layer, s_data.unitsSpeedOrHeartRate, font_18, GTextAlignmentCenter, COLOR_SPEED_UNITS, BG_COLOR_SPEED_UNITS, s_data.page_speed);

  // s_data.screenA_layer.field_top.data_layer NOT used
  // END top


  // BEGIN bottom left "distance"
  s_data.screenA_layer.field_bottom_left.title_layer = text_layer_create(GRect(PAGE_OFFSET_X + 1, PAGE_SPEED_TOP_H + 2, PAGE_W / 2 - 2, 16));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_left.title_layer, "distance", font_12, GTextAlignmentCenter, COLOR_TITLE, BG_COLOR_TITLE, s_data.page_speed);

  s_data.screenA_layer.field_bottom_left.unit_layer = text_layer_create(GRect(PAGE_OFFSET_X + 1, PAGE_SPEED_TOP_H + 48, PAGE_W / 2 - 2, 14));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_left.unit_layer, s_data.unitsDistance, font_12, GTextAlignmentCenter, COLOR_UNITS, BG_COLOR_UNITS, s_data.page_speed);

  s_data.screenA_layer.field_bottom_left.data_layer = text_layer_create(GRect(PAGE_OFFSET_X + 1, PAGE_SPEED_TOP_H + 22, PAGE_W / 2 - 2, 26));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_left.data_layer, s_data.distance, font_22_24, GTextAlignmentCenter, COLOR_DATA, BG_COLOR_DATA, s_data.page_speed);
  // END bottom left

  // BEGIN bottom right "avg"
  s_data.screenA_layer.field_bottom_right.title_layer = text_layer_create(GRect(PAGE_OFFSET_X + PAGE_W / 2 + 2, PAGE_SPEED_TOP_H + 2, PAGE_W / 2 - 2, 16));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_right.title_layer, "avg speed", font_12, GTextAlignmentCenter, COLOR_TITLE, BG_COLOR_TITLE, s_data.page_speed);

  s_data.screenA_layer.field_bottom_right.unit_layer = text_layer_create(GRect(PAGE_OFFSET_X + PAGE_W / 2 + 2, PAGE_SPEED_TOP_H + 48, PAGE_W / 2 - 2, 15));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_right.unit_layer, s_data.unitsSpeed, font_12, GTextAlignmentCenter, COLOR_UNITS, BG_COLOR_UNITS, s_data.page_speed);

  s_data.screenA_layer.field_bottom_right.data_layer = text_layer_create(GRect(PAGE_OFFSET_X + PAGE_W / 2 + 2, PAGE_SPEED_TOP_H + 22, PAGE_W / 2 - 2, 26));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_right.data_layer, s_data.avgspeed, font_22_24, GTextAlignmentCenter, COLOR_DATA, BG_COLOR_DATA, s_data.page_speed);
  // END bottom right

  layer_set_hidden(s_data.page_speed, false);
  //vibes_double_pulse();
}

void screen_speed_deinit() {
  for(int n=0; n < TOTAL_IMAGE_SLOTS; n++) {
    if(image_slots[n] != NOT_USED) {
      gbitmap_destroy(images[n]);
      bitmap_layer_destroy(image_layers[n]);
    }
  }
  
  layer_destroy(s_data.screenA_layer.speed_layer.layer);
  layer_destroy(s_data.page_speed);
  text_layer_destroy(s_data.screenA_layer.field_top.unit_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_left.title_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_left.data_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_left.unit_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_right.title_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_right.data_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_right.unit_layer);
  layer_destroy(line_layer);
}

void screen_speed_show_speed(bool force_units) {
  /*
  TODO(config)
  if (s_data.page_number != PAGE_SPEED && s_data.page_number != PAGE_HEARTRATE) {
    // nothing to do here
    return;
  }*/
  if (s_data.page_number == PAGE_HEARTRATE
#if ROTATION
   || rotation == ROTATION_HEARTRATE
#endif
   ) {
    if (force_units) {
      screen_speed_update_config();
      text_layer_set_text(s_data.screenA_layer.field_top.unit_layer, HEART_RATE_UNIT);
    }
    speed_layer_set_text(&s_data.screenA_layer.speed_layer, s_data.heartrate);
#if ROTATION  
  } else if (rotation == ROTATION_ALTITUDE) {
    snprintf(s_data.speed, sizeof(s_data.speed), "%d", s_gpsdata.altitude);
    if (force_units) {
      strncpy(s_data.unitsSpeedOrHeartRate, s_data.unitsAltitude, 8);
    }
#endif
  } else {
    copy_speed(s_data.speed, sizeof(s_data.speed), s_gpsdata.speed100);
    if (force_units) {
      screen_speed_update_config();
    }
  }
  copy_speed(s_data.maxspeed, sizeof(s_data.maxspeed), s_gpsdata.maxspeed100);
}
#if ROTATION
static void rotation_timer_callback(void *data) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "rotation_timer");
  rotation_timer = app_timer_register(2000, rotation_timer_callback, NULL);
  rotation++;
  if (rotation == ROTATION_HEARTRATE && s_gpsdata.heartrate == 255) {
    rotation++;
  }
  if (rotation == ROTATION_MAX) {
    rotation = ROTATION_MIN;
  }
   if (rotation == ROTATION_MIN) {
    s_data.screenA_layer.field_top.type = FIELD_SPEED;
    s_data.screenA_layer.field_bottom_left.type = FIELD_DISTANCE;
    s_data.screenA_layer.field_bottom_right.type = FIELD_AVGSPEED;
  } else {
    s_data.screenA_layer.field_top.type = FIELD_ALTITUDE;
    s_data.screenA_layer.field_bottom_left.type = FIELD_ASCENT;
    s_data.screenA_layer.field_bottom_right.type = FIELD_DISTANCE;
  }
  screen_speed_show_speed(true);
  screen_speed_update_config();
  update_screens();
}
void screen_speed_start_rotation() {
  if (rotation_timer == NULL) {
    rotation_timer = app_timer_register(500, rotation_timer_callback, NULL);
  } else {
    app_timer_cancel(rotation_timer);
    rotation_timer = NULL;
    rotation = ROTATION_DISABLED;
  }
}
#endif


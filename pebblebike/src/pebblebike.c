#include "pebble.h"
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "pebblebike.h"
#include "communication.h"
#include "buttons.h"
#include "menu.h"
#include "screens.h"
#include "screen_speed.h"
#include "screen_altitude.h"
#include "screen_live.h"
#include "screen_map.h"
#include "screen_config.h"
#if DEBUG
  #include "screen_debug.h"
#endif

GFont font_12, font_18, font_22_24;

AppData s_data;
GPSData s_gpsdata;
LiveData s_live;
int nbchange_state=0;

void change_units(uint8_t units, bool first_time) {
  if ((units == s_gpsdata.units) && !first_time) {
    return;
  }

  if (first_time) {
    if (persist_exists(PERSIST_UNITS_KEY)) {
      units = persist_read_int(PERSIST_UNITS_KEY);
    } else {
      // default value
      units = UNITS_METRIC;
    }
  } else {
    // save new value
    persist_write_int(PERSIST_UNITS_KEY, units);
  }

  s_gpsdata.units = units;
  switch(s_gpsdata.units) {
    case UNITS_IMPERIAL:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_IMPERIAL, 8);
      break;
    case UNITS_METRIC:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_METRIC, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_METRIC, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_METRIC, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_METRIC, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_METRIC, 8);
      break;
    case UNITS_NAUTICAL_IMPERIAL:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_NAUTICAL, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_NAUTICAL, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_IMPERIAL, 8);
      break;
    case UNITS_NAUTICAL_METRIC:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_NAUTICAL, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_NAUTICAL, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_METRIC, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_METRIC, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_METRIC, 8);
      break;
    case UNITS_RUNNING_IMPERIAL:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_RUNNING_IMPERIAL, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_IMPERIAL, 8);
      break;
    case UNITS_RUNNING_METRIC:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_RUNNING_METRIC, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_METRIC, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_METRIC, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_METRIC, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_METRIC, 8);
      break;
  }

  if (!first_time) {
    //todo(custom) screen_speed_dirty
    layer_mark_dirty(text_layer_get_layer(s_data.screenA_layer.field_top.unit_layer));
    layer_mark_dirty(text_layer_get_layer(s_data.screenA_layer.field_bottom_left.unit_layer));
    layer_mark_dirty(text_layer_get_layer(s_data.screenA_layer.field_bottom_right.unit_layer));
  }
  if (s_data.page_number == PAGE_SPEED) {
    strncpy(s_data.unitsSpeedOrHeartRate, s_data.unitsSpeed, 8);
  }
}

void change_state(uint8_t state) {
  if (state == s_data.state) {
    return;
  }
  s_data.state = state;
  if (s_data.state == STATE_STOP) {
    screen_reset_instant_data();
  }
  buttons_update();
  
  nbchange_state++;
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {

  char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(s_data.time, sizeof(s_data.time), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (s_data.time[0] == '0')) {
    memmove(s_data.time, &s_data.time[1], sizeof(s_data.time) - 1);
  }
  layer_mark_dirty(s_data.topbar_layer.layer);
}

void bt_callback(bool connected) {
  topbar_toggle_bluetooth_icon(connected);
}

static void init(void) {

  config_load();

  s_data.phone_battery_level = -1;

  font_12 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_12));
  font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_18));
# ifdef PBL_PLATFORM_APLITE
  font_22_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_24));
# else
  font_22_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_22));
# endif
  s_gpsdata.heartrate = 255; // no data at startup

  // set default unit of measure
  change_units(UNITS_IMPERIAL, true);
  
  buttons_init();

  s_data.window = window_create();
  //window_set_background_color(s_data.window, GColorBlue);
  window_set_background_color(s_data.window, GColorWhite);
# ifdef PBL_PLATFORM_APLITE
  window_set_fullscreen(s_data.window, true);
# endif
  topbar_layer_init(s_data.window);

  strcpy(s_data.speed, "0.0");
  strcpy(s_data.distance, "-");
  strcpy(s_data.avgspeed, "-");
  strcpy(s_data.altitude, "-");
  strcpy(s_data.ascent, "-");
  strcpy(s_data.ascentrate, "-");
  strcpy(s_data.slope, "-");
  strcpy(s_data.accuracy, "-");
  strcpy(s_data.bearing, "-");
  strcpy(s_data.elapsedtime, "00:00:00");
  strcpy(s_data.maxspeed, "-");
  //strcpy(s_data.lat, "-");
  //strcpy(s_data.lon, "-");
  //strcpy(s_data.nbascent, "-");
  strcpy(s_data.heartrate, "-");
  strcpy(s_data.cadence, "-");

  screen_speed_layer_init(s_data.window);
  screen_altitude_layer_init(s_data.window);
  screen_live_layer_init(s_data.window);
  screen_map_layer_init(s_data.window);

  #if DEBUG
    screen_debug1_layer_init(s_data.window);
    screen_debug2_layer_init(s_data.window);
  #endif

  screen_reset_instant_data();

  action_bar_init(s_data.window);
  menu_init();

  // Reduce the sniff interval for more responsive messaging at the expense of
  // increased energy consumption by the Bluetooth module
  // The sniff interval will be restored by the system after the app has been
  // unloaded
  //app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  
  communication_init();

  screen_speed_update_config();
  screen_altitude_update_config();
  
  window_stack_push(s_data.window, true /* Animated */);
  
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  bluetooth_connection_service_subscribe(bt_callback);
  
  send_version();
}
static void deinit(void) {
  communication_deinit();
  
  window_destroy(s_data.window);

  topbar_layer_deinit();

  screen_speed_deinit();
  screen_altitude_layer_deinit();
  screen_live_layer_deinit();
  screen_map_layer_deinit();

  #if DEBUG
    screen_debug1_layer_deinit();
    screen_debug2_layer_deinit();
  #endif
  
  action_bar_deinit();
  menu_deinit();

  buttons_deinit();

  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

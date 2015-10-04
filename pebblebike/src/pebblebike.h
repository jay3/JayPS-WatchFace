#ifndef PEBBLEBIKE_H
#define PEBBLEBIKE_H

enum {
    MSG_LOCATION_DATA = 0x13, // TUPLE_BYTE_ARRAY
    STATE_CHANGED = 0x14,
    MSG_VERSION_PEBBLE = 0x15,
    MSG_VERSION_ANDROID = 0x16,
    MSG_LIVE_SHORT = 0x17,
    //MSG_LIVE_ASK_NAMES = 0x18,
    MSG_LIVE_NAME0 = 0x19,
    MSG_LIVE_NAME1 = 0x20,
    MSG_LIVE_NAME2 = 0x21,
    MSG_LIVE_NAME3 = 0x22,
    MSG_LIVE_NAME4 = 0x23,
    MSG_BATTERY_LEVEL = 0x24,
    MSG_LOCATION_DATA_V2 = 0x25,
    MSG_LOCATION_DATA_V3 = 0x26,
    MSG_SENSOR_TEMPERATURE = 0x27,
};

enum {
    STATE_STOP = 0,
    STATE_START = 1,
};

enum {
    PLAY_PRESS = 0x0,
    STOP_PRESS = 0x1,
    REFRESH_PRESS = 0x2,
    CMD_BUTTON_PRESS = 0x4,

    ORUXMAPS_START_RECORD_CONTINUE_PRESS = 0x5,
    ORUXMAPS_STOP_RECORD_PRESS = 0x6,
    ORUXMAPS_NEW_WAYPOINT_PRESS = 0x7,
};

enum {
    UNITS_IMPERIAL = 0x0,
    UNITS_METRIC = 0x1,
    UNITS_NAUTICAL_IMPERIAL = 0x2,
    UNITS_NAUTICAL_METRIC = 0x3,
    UNITS_RUNNING_IMPERIAL = 0x4,
    UNITS_RUNNING_METRIC = 0x5,
};

enum {
    PAGE_SPEED = 0,
    PAGE_HEARTRATE = 1,
    PAGE_ALTITUDE = 2,
    PAGE_LIVE_TRACKING = 3,
    PAGE_MAP = 4,
    PAGE_DEBUG1 = 5,
    PAGE_DEBUG2 = 6,
};
#define PAGE_FIRST PAGE_SPEED
enum {
    PERSIST_UNITS_KEY = 0x0,
    PERSIST_CONFIG_KEY = 0x1,
};

#if DEBUG
#define NUMBER_OF_PAGES 7
#endif

#if !DEBUG
#define NUMBER_OF_PAGES 5
#endif

#define CHAR_WIDTH 35
#define DOT_WIDTH 15
#define CHAR_HEIGHT 51



#define TOPBAR_HEIGHT 18
#ifdef PBL_PLATFORM_CHALK
  #define SCREEN_W 180
  #define SCREEN_H 180
  // 18
  #define PAGE_OFFSET_X ((180-144)/2)
  // 6
  //#define PAGE_OFFSET_Y ((180-168)/2 - TOPBAR_HEIGHT)
  #define PAGE_OFFSET_Y TOPBAR_HEIGHT
#else
  #define SCREEN_W 144
  #define SCREEN_H 168
  #define PAGE_OFFSET_X 0
  #define PAGE_OFFSET_Y TOPBAR_HEIGHT
#endif


#ifdef PBL_PLATFORM_APLITE
  // 20+2
  #define MENU_WIDTH (ACTION_BAR_WIDTH+2)
#else
  #ifdef PBL_PLATFORM_CHALK
    // 40 - 18 = 22
    //#define MENU_WIDTH (ACTION_BAR_WIDTH-PAGE_OFFSET_X)
    #define MENU_WIDTH 0
  #else
    // 30
    #define MENU_WIDTH (ACTION_BAR_WIDTH)
  #endif
#endif

#define PAGE_W  (144-MENU_WIDTH)
#define PAGE_H (SCREEN_H-TOPBAR_HEIGHT)
//#define PAGE_GRECT GRect(PAGE_OFFSET_X, PAGE_OFFSET_Y, PAGE_W, PAGE_H)
#define PAGE_GRECT GRect(0, PAGE_OFFSET_Y, SCREEN_W, PAGE_H)
#define SCREEN_GRECT GRect(0, 0, SCREEN_W, SCREEN_H)


#define SPEED_UNIT_IMPERIAL "mph"
#define SPEED_UNIT_METRIC "km/h"
#define SPEED_UNIT_NAUTICAL "kn"
#define SPEED_UNIT_RUNNING_IMPERIAL "min/m"
#define SPEED_UNIT_RUNNING_METRIC "min/km"
#define DISTANCE_UNIT_IMPERIAL "miles"
#define DISTANCE_UNIT_METRIC "km"
#define DISTANCE_UNIT_NAUTICAL "nm"
#define ALTITUDE_UNIT_METRIC "m"
#define ALTITUDE_UNIT_IMPERIAL "ft"
#define ASCENT_RATE_UNIT_METRIC "m/h"
#define ASCENT_RATE_UNIT_IMPERIAL "ft/h"
#define HEART_RATE_UNIT "bpm"
#define TEMPERATURE_UNIT_METRIC "°C"
#define TEMPERATURE_UNIT_IMPERIAL "°F"
//TODO: don't use field values to sort (only to save - persistent)
enum {
    FIELD__MIN,
    FIELD_ACCURACY = FIELD__MIN,
    FIELD_ALTITUDE,
    FIELD_ASCENT,
    FIELD_ASCENTRATE,
    FIELD_AVGSPEED,
    FIELD_BEARING,
    FIELD_CADENCE,
    FIELD_DISTANCE,
    FIELD_DURATION,
    FIELD_HEARTRATE,
    //FIELD_LAT,
    //FIELD_LON,
    FIELD_MAXSPEED,
    //FIELD_NBASCENT,
    FIELD_SLOPE,
    FIELD_SPEED,
    FIELD_TEMPERATURE,
    //FIELD_TIME,
    FIELD__MAX,
};
typedef struct TopBarLayer {
    Layer *layer;
    TextLayer *time_layer;
    TextLayer *accuracy_layer;
    GBitmap *bluetooth_image;
    BitmapLayer *bluetooth_layer;
} TopBarLayer;

typedef struct SpeedLayer {
    Layer *layer;
    char* text;
} SpeedLayer;

typedef struct FieldLayer {
    uint8_t type;
    Layer *main_layer;
    TextLayer *title_layer;
    TextLayer *data_layer;
    TextLayer *unit_layer;
    char title[12];
    char units[8];
} FieldLayer;

typedef struct ScreenALayer {
    FieldLayer field_top;
    FieldLayer field_bottom_left;
    FieldLayer field_bottom_right;
    SpeedLayer speed_layer;
} ScreenALayer;
typedef struct ScreenBLayer {
    FieldLayer field_top_left;
    FieldLayer field_top_right;
    FieldLayer field_bottom_left;
    FieldLayer field_bottom_right;
} ScreenBLayer;

typedef struct AppData {
    Window *window;

    Layer *page_speed;
    Layer *page_altitude;
    MenuLayer *page_live_tracking;
    Layer *page_map;

#if DEBUG
    Layer *page_debug1;
    Layer *page_debug2;
#endif

    TopBarLayer topbar_layer;

    ScreenALayer screenA_layer;
    ScreenBLayer screenB_layer;

    //FieldLayer altitude_accuracy;

    TextLayer *live_tracking_layer;
#if DEBUG
    TextLayer *debug1_layer;
    TextLayer *debug2_layer;
#endif

    char time[6]; // xx:xx, \0 terminated
    char speed[16];
    char distance[6];
    char avgspeed[6];
    char altitude[6];
    char ascent[8];
    char ascentrate[8];
    char slope[8];
    char accuracy[5];
    char bearing[4];
    char elapsedtime[8];
    char maxspeed[8];
    //char lat[8];
    //char lon[8];
    //char nbascent[8];
    char heartrate[8];
    char cadence[8];
    char temperature[7];

#if DEBUG
    char debug1[200];
    char debug2[200];
#endif
    char unitsSpeedOrHeartRate[8];
    char unitsSpeed[8];
    char unitsDistance[8];
    char unitsAltitude[8];
    char unitsAscentRate[8];
    char unitsTemperature[8];
    uint8_t state;
    uint8_t live;
    uint8_t debug;
    uint8_t refresh_code;
    int page_number;

    int32_t android_version;
    int32_t phone_battery_level;
} AppData;

typedef struct GPSData {
    int nb_received;
    uint8_t units;
    uint16_t time;
    int32_t speed100;
    int32_t maxspeed100;
    int32_t distance100;
    int32_t avgspeed100;
    int16_t altitude;
    int16_t ascent;
    int16_t ascentrate;

    int8_t slope;
    uint8_t accuracy;
    int16_t xpos;
    int16_t ypos;
    uint16_t bearing;
    uint8_t heartrate;
    uint8_t cadence;
    int16_t temperature10;
} GPSData;


//////////////
// Live Data
//////////////
#define NUM_LIVE_FRIENDS 5
typedef struct LiveFriendData {
    char name[10];
    char subtitle[20];
    int16_t xpos;
    int16_t ypos;
    int32_t distance;
    uint16_t bearing;
    int16_t lastviewed;
    TextLayer *name_layer;
    GRect name_frame;
} LiveFriendData;
typedef struct LiveData {
    uint8_t nb;
    LiveFriendData friends[NUM_LIVE_FRIENDS];
    LiveFriendData *sorted_friends[NUM_LIVE_FRIENDS];
} LiveData;

extern GFont font_12, font_18, font_22_24;
extern AppData s_data;
extern GPSData s_gpsdata;
extern LiveData s_live;

#if DEBUG
  extern char tmp[255];
#endif

void change_units(uint8_t units, bool first_time);
void change_state(uint8_t state);

#endif // PEBBLEBIKE_H

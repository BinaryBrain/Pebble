#include <pebble.h>

static Window* screen_window;
static Window* menu_window;
TextLayer* top_layer;
TextLayer* bot_layer;
SimpleMenuLayer* menu_layer;

#define false 0
#define true 1

// Android Communication
#define REQUEST_LOCATION                0
#define REQUEST_FIX_LOCATION            1
#define REQUEST_START_THREADED_LOCATION 2
#define REQUEST_STOP_THREADED_LOCATION  3
#define REQUEST_ELEVATION               4
#define REQUEST_WEATHER_STATUS          5
#define REQUEST_WEATHER_TEMPERATURE     6
#define REQUEST_WEATHER_PRESSURE        7
#define REQUEST_WEATHER_HUMIDITY        8
#define REQUEST_WEATHER_WIND            9
#define REQUEST_WEATHER_SUNRISE        10
#define REQUEST_WEATHER_SUNSET         11
#define REQUEST_TRANSPORT              12
#define SHOW_UP_TIME                   13
#define SHOW_ACTIVE_TIME               14
#define SHOW_BATTERY_STATE             15

#define NUMBER_OF_ITEMS                16


// Pebble KEY
#define PEBBLE_KEY_VALUE        1
// Location API
#define KEY_LATITUDE        100
#define KEY_LONGITUDE       101
#define KEY_DISTANCE        102
#define KEY_DIRECTION       103
// Elevation API
#define KEY_ALTITUDE        200
// Weather API
#define KEY_STATUS          300
#define KEY_DESCRIPTION     301
#define KEY_TEMPERATURE     302
#define KEY_PRESSURE        303
#define KEY_HUMIDITY        304
#define KEY_WIND_SPEED      305
#define KEY_WIND_DIRECTION  306
#define KEY_SUNRISE         307
#define KEY_SUNSET          308
// Transport API
#define KEY_DEPARTURE       400
#define KEY_DEPARTURE_TIME  401
#define KEY_ARRIVAL         402
#define KEY_ARRIVAL_TIME    403


#define MAX_TEXT_SIZE       128
#define NUM_ACCEL_SAMPLES   10
#define GRAVITY             10000 // (1g)² = 10000
#define ACCEL_THRESHOLD     8000  // (1g)² = 10000
#define NUMBER_OF_SCREENS   4

#define NUMBER_OF_SECTIONS        5
#define NUMBER_OF_LOCATION_ITEMS  5
#define NUMBER_OF_WEATHER_ITEMS   7
#define NUMBER_OF_TRANSPORT_ITEMS 1
#define NUMBER_OF_TIME_ITEMS      2
#define NUMBER_OF_BATTERY_ITEMS   1


int counter = -1;
char text[MAX_TEXT_SIZE];
unsigned long int up_time = 0;      //in seconds
unsigned long int active_time = 0;  //in seconds/10

typedef enum { SCREEN_NAV, SCREEN_HIGHLIGHT, SCREEN_EDIT } state_type;
state_type state = SCREEN_NAV;

typedef struct ScreenPart {
  int type;
  char text[500];
} ScreenPart;

typedef struct Screen {
  ScreenPart topPart;
  ScreenPart botPart;
} Screen;

Screen screens[NUMBER_OF_SCREENS];
int currentScreen = 0;

void up_click_handler(ClickRecognizerRef recognizer, void *context);
void mid_click_handler(ClickRecognizerRef recognizer, void *context);
void down_click_handler(ClickRecognizerRef recognizer, void *context);
void back_click_handler(ClickRecognizerRef recognizer, void *context);
void highlight();
void clearHighlight();
void initMenu();
void showMenu();
void hideMenu();
void menu_select_callback(int index, void *ctx);
void click_config_provider(void *context);
void click_config_provider_menu(void *context);
void getSelectionText(int index, char* text);
void send(int key, char *value);
void updateScreen(const char* string, int index);

/* SCREEN NAV */
void initScreen() {
  ScreenPart top0 = { .text = "Sélectionnez un type de donnée...", .type = 0 };
  ScreenPart bot0 = { .text = "Sélectionnez un type de donnée...", .type = 0 };
  screens[0].topPart = top0;
  screens[0].botPart = bot0;
  ScreenPart top1 = { .text = "Sélectionnez un type de donnée...", .type = 0 };
  ScreenPart bot1 = { .text = "Sélectionnez un type de donnée...", .type = 0 };
  screens[1].topPart = top1;
  screens[1].botPart = bot1;
  ScreenPart top2 = { .text = "Sélectionnez un type de donnée...", .type = 0 };
  ScreenPart bot2 = { .text = "Sélectionnez un type de donnée...", .type = 0 };
  screens[2].topPart = top2;
  screens[2].botPart = bot2;
  ScreenPart top3 = { .text = "Sélectionnez un type de donnée...", .type = 0 };
  ScreenPart bot3 = { .text = "Sélectionnez un type de donnée...", .type = 0 };
  screens[3].topPart = top3;
  screens[3].botPart = bot3;
}

void refreshScreen() {
  text_layer_set_text(top_layer, screens[currentScreen].topPart.text);
  text_layer_set_text(bot_layer, screens[currentScreen].botPart.text);
}

void changeScreenUp() {
  currentScreen += 1;
  if (currentScreen >= NUMBER_OF_SCREENS) {
    currentScreen = 0;
  }

  refreshScreen();
}

void changeScreenDown() {
  currentScreen -= 1;
  if (currentScreen < 0) {
    currentScreen = NUMBER_OF_SCREENS-1;
  }

  refreshScreen();
}

/* SCREEN HIGHLIGHT */
bool topHighlighted = true;
void highlight() {
  clearHighlight();
  
  if(topHighlighted) {
    text_layer_set_background_color(top_layer, GColorBlack);
    text_layer_set_text_color(top_layer, GColorWhite);
    topHighlighted = false;
  } else {
    text_layer_set_background_color(bot_layer, GColorBlack);
    text_layer_set_text_color(bot_layer, GColorWhite);
    topHighlighted = true;
  }
}

void clearHighlight() {
  text_layer_set_background_color(top_layer, GColorWhite);
  text_layer_set_text_color(top_layer, GColorBlack);
  text_layer_set_background_color(bot_layer, GColorWhite);
  text_layer_set_text_color(bot_layer, GColorBlack);
}

/* SCREEN EDIT */
static void menu_select_callback_location(int index, void *ctx) {
  menu_select_callback(index, ctx);
}
static void menu_select_callback_weather(int index, void *ctx) {
  menu_select_callback(index + NUMBER_OF_LOCATION_ITEMS, ctx);
}
static void menu_select_callback_transport(int index, void *ctx) {
  menu_select_callback(index + NUMBER_OF_LOCATION_ITEMS + NUMBER_OF_WEATHER_ITEMS, ctx);
}
static void menu_select_callback_time(int index, void *ctx) {
  menu_select_callback(index + NUMBER_OF_LOCATION_ITEMS + NUMBER_OF_WEATHER_ITEMS + NUMBER_OF_TRANSPORT_ITEMS, ctx);
}
static void menu_select_callback_battery(int index, void *ctx) {
  menu_select_callback(index + NUMBER_OF_LOCATION_ITEMS + NUMBER_OF_WEATHER_ITEMS + NUMBER_OF_TRANSPORT_ITEMS + NUMBER_OF_TIME_ITEMS, ctx);
}

void menu_select_callback(int index, void *ctx) {
  char text[200] = "";
  getSelectionText(index, text);
  
  if(topHighlighted) { // Reverted
    strcpy(screens[currentScreen].botPart.text, text);
    screens[currentScreen].botPart.type = index;
    layer_mark_dirty(text_layer_get_layer(bot_layer));
  } else {
    strcpy(screens[currentScreen].topPart.text, text);
    screens[currentScreen].topPart.type = index;
    layer_mark_dirty(text_layer_get_layer(top_layer));
  }
  
  refreshScreen();
  
  hideMenu();
  state = SCREEN_NAV;
  clearHighlight();
  topHighlighted = true;
  
  send(index, "");
  int isBot = (topHighlighted) ? 0 : 1;
  persist_write_int(currentScreen * 2 + isBot, index);
}

SimpleMenuSection menuSections[NUMBER_OF_SECTIONS];
// Sections
SimpleMenuItem locationItems[NUMBER_OF_LOCATION_ITEMS];
SimpleMenuItem weatherItems[NUMBER_OF_WEATHER_ITEMS];
SimpleMenuItem transportItems[NUMBER_OF_TRANSPORT_ITEMS];
SimpleMenuItem timeItems[NUMBER_OF_TIME_ITEMS];
SimpleMenuItem batteryItems[NUMBER_OF_BATTERY_ITEMS];

void initMenu() {
  menuSections[0] = (SimpleMenuSection) {
    .title = "Location",
    .num_items = NUMBER_OF_LOCATION_ITEMS,
    .items = locationItems,
  };
  menuSections[1] = (SimpleMenuSection) {
    .title = "Weather",
    .num_items = NUMBER_OF_WEATHER_ITEMS,
    .items = weatherItems,
  };
  menuSections[2] = (SimpleMenuSection) {
    .title = "Transport",
    .num_items = NUMBER_OF_TRANSPORT_ITEMS,
    .items = transportItems,
  };
  menuSections[3] = (SimpleMenuSection) {
    .title = "Time",
    .num_items = NUMBER_OF_TIME_ITEMS,
    .items = timeItems,
  };
  menuSections[4] = (SimpleMenuSection) {
    .title = "Battery",
    .num_items = NUMBER_OF_BATTERY_ITEMS,
    .items = batteryItems,
  };
  
  // Items
  locationItems[0] = (SimpleMenuItem) {
    .title = "Location",
    .callback = menu_select_callback_location,
  };
  locationItems[1] = (SimpleMenuItem) {
    .title = "Fixing Target",
    .callback = menu_select_callback_location,
  };
  locationItems[2] = (SimpleMenuItem) {
    .title = "Start Thread Navigation",
    .callback = menu_select_callback_location,
  };
  locationItems[3] = (SimpleMenuItem) {
    .title = "Stop Thread Navigationt",
    .callback = menu_select_callback_location,
  };
  locationItems[4] = (SimpleMenuItem) {
    .title = "Elevation",
    .callback = menu_select_callback_location,
  };
  
  weatherItems[0] = (SimpleMenuItem) {
    .title = "Weather Status",
    .callback = menu_select_callback_weather,
  };
  weatherItems[1] = (SimpleMenuItem) {
    .title = "Temparature",
    .callback = menu_select_callback_weather,
  };
  weatherItems[2] = (SimpleMenuItem) {
    .title = "Pressure",
    .callback = menu_select_callback_weather,
  };
  weatherItems[3] = (SimpleMenuItem) {
    .title = "Humidity",
    .callback = menu_select_callback_weather,
  };
  weatherItems[4] = (SimpleMenuItem) {
    .title = "Wind",
    .callback = menu_select_callback_weather,
  };
  weatherItems[5] = (SimpleMenuItem) {
    .title = "Sunrise",
    .callback = menu_select_callback_weather,
  };
  weatherItems[6] = (SimpleMenuItem) {
    .title = "Sunset",
    .callback = menu_select_callback_weather,
  };
  
  transportItems[0] = (SimpleMenuItem) {
    .title = "Transport",
    .callback = menu_select_callback_transport,
  };
  
  timeItems[0] = (SimpleMenuItem) {
    .title = "Uptime",
    .callback = menu_select_callback_time,
  };
  timeItems[1] = (SimpleMenuItem) {
    .title = "Active Time",
    .callback = menu_select_callback_time,
  };
  
  batteryItems[0] = (SimpleMenuItem) {
    .title = "Battery Status",
    .callback = menu_select_callback_battery,
  };
}

// Use
void showMenu() {
  window_stack_push(menu_window, true);
  if (topHighlighted) {
    simple_menu_layer_set_selected_index(menu_layer, screens[currentScreen].botPart.type, true);
  } else {
    simple_menu_layer_set_selected_index(menu_layer, screens[currentScreen].topPart.type, true);
  }
}

void hideMenu() {
  window_stack_remove(menu_window, true);
}

void send(int key, char *value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_cstring(iter, key, value);
  app_message_outbox_send();
}

void updateScreen(const char* string, int index) {
  for (unsigned int i = 0; i < NUMBER_OF_SCREENS; i++) {
    if (screens[i].topPart.type == index) {
      strcpy(screens[i].topPart.text, string);
    }
    if (screens[i].botPart.type == index) {
      strcpy(screens[i].botPart.text, string);
    }
  }
  
  refreshScreen();
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Get time since launch
  int seconds = up_time % 60;
  int minutes = (up_time % 3600) / 60;
  int hours = up_time / 3600;

  snprintf(text, MAX_TEXT_SIZE, "Uptime:\n%dh %dm %ds", hours, minutes, seconds);

  updateScreen(text, SHOW_UP_TIME);
  
  // Battery
  BatteryChargeState charge_state = battery_state_service_peek();

  if (charge_state.is_charging) {
    snprintf(text, MAX_TEXT_SIZE, "Battery is charging");
  }
  else {
    snprintf(text, MAX_TEXT_SIZE, "Battery is\n%d%% charged", charge_state.charge_percent);
  }
  updateScreen(text, SHOW_BATTERY_STATE);
  
  // Increment uptime
  up_time++;
}

static void data_handler(AccelData *data, uint32_t num_samples) {  // accel from -4000 to 4000, 1g = 1000 cm/s²
  int i, x, y, z, acc_norm_2;
  for (i = 0; i < NUM_ACCEL_SAMPLES; i++) {
    // Divide by 10 to avoid too high values. Now from -400 to 400
    x = data[i].x / 10;                    // accel in dm/s²
    y = data[i].y / 10;                    // accel in dm/s²
    z = data[i].z / 10;                    // accel in dm/s²
    // 1g = 100 dm/s²  
    acc_norm_2 = (x*x) + (y*y) + (z*z);    // (1g)² = 10000
    //APP_LOG(APP_LOG_LEVEL_INFO, "%d %d %d %d", x, y, z, acc_norm_2);
    if ( ((acc_norm_2 - GRAVITY) > ACCEL_THRESHOLD) || ((GRAVITY - acc_norm_2) > ACCEL_THRESHOLD) ) {
      active_time++;
    }
  }

  int active_time_s = active_time / 10;
  int seconds = active_time_s % 60;
  int minutes = (active_time_s % 3600) / 60;
  int hours = active_time_s / 3600;

  snprintf(text, MAX_TEXT_SIZE, "Active time:\n%dh %dm %ds", hours, minutes, seconds);

  updateScreen(text, SHOW_ACTIVE_TIME);
}

void received_handler(DictionaryIterator *iter, void *context) {
  Tuple *result_tuple = dict_find(iter, PEBBLE_KEY_VALUE);
  switch(result_tuple->value->int32) {
    // Location API
    case REQUEST_LOCATION:
      strcpy(text, "lat : ");
      strcat(text, dict_find(iter, KEY_LATITUDE)->value->cstring);
      strcat(text, "\nlon : ");
      strcat(text, dict_find(iter, KEY_LONGITUDE)->value->cstring);
      break;
    case REQUEST_START_THREADED_LOCATION:
      strcpy(text, "distance : ");
      strcat(text, dict_find(iter, KEY_DISTANCE)->value->cstring);
      strcat(text, "\ndirection : ");
      strcat(text, dict_find(iter, KEY_DIRECTION)->value->cstring);
      break;
    // Elevation API
    case REQUEST_ELEVATION:
      strcpy(text, "altitude : ");
      strcat(text, dict_find(iter, KEY_ALTITUDE)->value->cstring);
      strcat(text, "m");
      break;
    // Weather API
    case REQUEST_WEATHER_STATUS:
      strcpy(text, dict_find(iter, KEY_STATUS)->value->cstring);
      strcat(text, "\n");
      strcat(text, dict_find(iter, KEY_DESCRIPTION)->value->cstring);
      break;
    case REQUEST_WEATHER_TEMPERATURE:
      strcpy(text, dict_find(iter, KEY_TEMPERATURE)->value->cstring);
      strcat(text, "°C");
      break;
    case REQUEST_WEATHER_PRESSURE:
      strcpy(text, "pressure : ");
      strcat(text, dict_find(iter, KEY_PRESSURE)->value->cstring);
      break;
    case REQUEST_WEATHER_HUMIDITY:
      strcpy(text, "humidity : ");
      strcat(text, dict_find(iter, KEY_HUMIDITY)->value->cstring);
      break;
    case REQUEST_WEATHER_WIND:
      strcpy(text, "wind speed : ");
      strcat(text, dict_find(iter, KEY_WIND_SPEED)->value->cstring);
      strcat(text, "km/h\nwind direction : ");
      strcat(text, dict_find(iter, KEY_WIND_DIRECTION)->value->cstring);
      break;
    case REQUEST_WEATHER_SUNRISE:
      strcpy(text, "sunrise : \n");
      strcat(text, dict_find(iter, KEY_SUNRISE)->value->cstring);
      break;
    case REQUEST_WEATHER_SUNSET:
      strcpy(text, "sunset : \n");
      strcat(text, dict_find(iter, KEY_SUNSET)->value->cstring);
      break;
    // Transport API
    case REQUEST_TRANSPORT:
      strcpy(text, dict_find(iter, KEY_DEPARTURE)->value->cstring);
      strcat(text, " : ");
      strcat(text, dict_find(iter, KEY_DEPARTURE_TIME)->value->cstring);
      strcat(text, "\n");
      strcat(text, dict_find(iter, KEY_ARRIVAL)->value->cstring);
      strcat(text, " : ");
      strcat(text, dict_find(iter, KEY_ARRIVAL_TIME)->value->cstring);
      break;
    default:
      strcpy(text, "Error.\nPlease check your dictionary KEYS");
      break;
  }
  
  updateScreen(text, result_tuple->value->int32);
}


void getSelectionText(int index, char* text) {
  switch (index) {
    case REQUEST_LOCATION:
      strcpy(text, "Request for:\nLOCATION\nsent");
      break;
    case REQUEST_FIX_LOCATION:
      strcpy(text, "Request for:\nFIXING TARGET\nsent");
      break;
    case REQUEST_START_THREADED_LOCATION:
      strcpy(text, "Request for:\nSTART THREAD NAVIGATION\nsent");
      break;
    case REQUEST_STOP_THREADED_LOCATION:
      strcpy(text, "Request for:\nSTOP THREAD NAVIGATION\nsent");
      break;
    case REQUEST_ELEVATION:
      strcpy(text, "Request for:\nELEVATION\nsent");
      break;
    case REQUEST_WEATHER_STATUS:
      strcpy(text, "Request for:\nWEATHER_STATUS\nsent");
      break;
    case REQUEST_WEATHER_TEMPERATURE:
      strcpy(text, "Request for:\nTEMPERATURE\nsent");
      break;
    case REQUEST_WEATHER_PRESSURE:
      strcpy(text, "Request for:\nPRESSURE\nsent");
      break;
    case REQUEST_WEATHER_HUMIDITY:
      strcpy(text, "Request for:\nHUMIDITY\nsent");
      break;
    case REQUEST_WEATHER_WIND:
      strcpy(text, "Request for:\nWIND\nsent");
      break;
    case REQUEST_WEATHER_SUNRISE:
      strcpy(text, "Request for:\nSUNRISE\nsent");
      break;
    case REQUEST_WEATHER_SUNSET:
      strcpy(text, "Request for:\nSUNSET\nsent");
      break;
    case REQUEST_TRANSPORT:
      strcpy(text, "Request for:\nTRANSPORT\nsent");
      break;
    case SHOW_UP_TIME:
      strcpy(text, "Mode:\nSHOW_UP_TIME\nset");
      break;
    case SHOW_ACTIVE_TIME:
      strcpy(text, "Mode:\nSHOW_ACTIVE_TIME\nset");
      break;
    case SHOW_BATTERY_STATE:
      strcpy(text, "Mode:\nSHOW_BATTERY_STATE\nset");
      break;
    default:
      strcpy(text, "Error.\nPlease check if NUMBER_OF_ITEMS is OK");
      break;
  }
}

static void screen_window_load(Window *window) {
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  top_layer = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h/2));
  bot_layer = text_layer_create(GRect(0, bounds.size.h/2, bounds.size.w, bounds.size.h/2));
  
  layer_add_child(window_layer, text_layer_get_layer(top_layer));
  layer_add_child(window_layer, text_layer_get_layer(bot_layer));
  
  // initScreen();
  // refreshScreen();
}

static void screen_window_unload(Window *window) {
  text_layer_destroy(top_layer);
  text_layer_destroy(bot_layer);
}

static void menu_window_load(Window *window) {
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  menu_layer = simple_menu_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h), window, menuSections, NUMBER_OF_SECTIONS, NULL);
  layer_add_child(window_layer, simple_menu_layer_get_layer(menu_layer));
  
  initMenu();
}

static void menu_window_unload(Window *window) {
  simple_menu_layer_destroy(menu_layer);
}

/**
 * Initializes
 */
static void init(void) {
  // Screen Window
  screen_window = window_create();
  window_set_click_config_provider(screen_window, click_config_provider);
  window_set_window_handlers(screen_window, (WindowHandlers) {
    .load = screen_window_load,
    .unload = screen_window_unload,
  });
  
  window_stack_push(screen_window, true);
  
  // Menu Window
  menu_window = window_create();
  window_set_window_handlers(menu_window, (WindowHandlers) {
    .load = menu_window_load,
    .unload = menu_window_unload,
  });
  
  window_set_click_config_provider(menu_window, click_config_provider_menu);
  // menu_layer_set_click_config_onto_window(simple_menu_layer_get_menu_layer(menu_layer), menu_window);
  
  // Subscribe to TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  // Subscribe to the accelerometer data service
  accel_data_service_subscribe(NUM_ACCEL_SAMPLES, data_handler);
  // Choose update rate
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);

  app_message_register_inbox_received(received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  for (unsigned int i = 0; i < NUMBER_OF_SCREENS * 2; i += 2) {
    int index = 0;
    if (persist_exists(i)) {
      index = persist_read_int(i);
          
      char text[200] = "";
      getSelectionText(index, text);
      
      strcpy(screens[i/2].topPart.text, text);
      screens[i/2].topPart.type = index;
      layer_mark_dirty(text_layer_get_layer(top_layer));
      
      refreshScreen();
      send(index, "");
    }
    
    if (persist_exists(i+1)) {
      index = persist_read_int(i+1);
          
      char text[200] = "";
      getSelectionText(index, text);
      
      strcpy(screens[(i-1)/2].botPart.text, text);
      screens[(i-1)/2].botPart.type = index;
      layer_mark_dirty(text_layer_get_layer(bot_layer));
      
      refreshScreen();
      send(index, "");
    }
  }
}

static void deinit(void) {
  window_destroy(screen_window);
  window_destroy(menu_window);
}

/* EVENTS */
void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, mid_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
	window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}


void back_click_handler_menu(ClickRecognizerRef recognizer, void *context) {
  state = SCREEN_HIGHLIGHT;
  hideMenu();
}

void click_config_provider_menu(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler_menu);
}

void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (state) {
    case SCREEN_NAV:
      changeScreenUp();
      break;
    case SCREEN_HIGHLIGHT:
      highlight();
      break;
    case SCREEN_EDIT:
      break;
  }
}

void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (state) {
    case SCREEN_NAV:
      changeScreenDown();
      break;
    case SCREEN_HIGHLIGHT:
      highlight();
      break;
    case SCREEN_EDIT:
      break;
  }
}

void mid_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (state) {
    case SCREEN_NAV:
      state = SCREEN_HIGHLIGHT;
      highlight();
      break;
    case SCREEN_HIGHLIGHT:
      state = SCREEN_EDIT;
      showMenu();
      break;
    case SCREEN_EDIT:
      break;
  }
}

void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (state) {
    case SCREEN_NAV:
      // TODO quit?
      break;
    case SCREEN_HIGHLIGHT:
      state = SCREEN_NAV;
      clearHighlight();
      topHighlighted = true;
      break;
    case SCREEN_EDIT:
      state = SCREEN_HIGHLIGHT;
      hideMenu();
      break;
  }
}

/**
 * Starts the Pebble app.
 */
int main(void) {
  init();
  app_event_loop();
  deinit();
}
#include <pebble.h>

#define KEY_TRENDS 0

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_trending_layer;
static GFont s_time_font;
static GFont s_trending_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static char trend_layer_buffer[32];

static void update_time() {
  // Get a tm struct
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Create a long-lived buffer
  static char buffer[] = "00:00";
  
  if(clock_is_24h_style() == true) {
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  if(tick_time->tm_min % 60 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    app_message_outbox_send();
  }
}

static void main_window_load(Window *window) {
  // Create Background Layer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  
  
  // Create time Textlayer
  s_time_layer = text_layer_create(GRect(5, 52, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  
  // Create trends TextLayer
  s_trending_layer = text_layer_create(GRect(0, 110, 144, 25));
  text_layer_set_background_color(s_trending_layer, GColorClear);
  text_layer_set_text_color(s_trending_layer, GColorWhite);
  text_layer_set_text(s_trending_layer, "Mewing...");
  
  // Time Font
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_CONDENSED_BOLD_42));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Trending Font
  s_trending_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_CONDENSED_LT_20));
  text_layer_set_font(s_trending_layer, s_trending_font);
  text_layer_set_text_alignment(s_trending_layer, GTextAlignmentCenter);

  //Add layers in order as child layers to the Window's root layer
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_trending_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_trending_layer);
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_trending_layer);
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);
  while(t != NULL ) {
    switch(t->key) {
      case KEY_TRENDS:
          snprintf(trend_layer_buffer, sizeof(trend_layer_buffer), "%s", t->value->cstring);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized", (int)t->key);
        break;
    }
    t = dict_read_next(iterator);
  }
  text_layer_set_text(s_trending_layer, trend_layer_buffer);  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message Dropped!");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Create main window element and assign to pointer
  s_main_window = window_create();
  
   // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register Message Callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  //Open Message
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  
  update_time();
}

static void deinit() {
  // Destroy window
  window_destroy(s_main_window);
}

int main(void) {
  
  
  init();
  app_event_loop();
  deinit();
}

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <User_Setup.h>

#include "img.h"
#include "utils/utils.h"
#include "utils/macros.h"
#include "wifi/wifi.h"
#include "paint/paint_app.h"
#include "music/music_app.h"
#include "clock/clock_timer.h"
#include "gallery/galery_app.h"
#include "weather/weather_app.h"
#include "keyboard/keyboard.h"
#include "remote_control/remote_controller.h"

#define ROTATION 2
#define ROTATION_2_CAL_PARAMS 4

TFT_eSPI tft = TFT_eSPI();

static uint32_t background_color_index = 0;
// static uint16_t cal_data[] = {354, 3684, 295, 3789, ROTATION_2_CAL_PARAMS};
static uint16_t cal_data[] = {300, 3500, 128, 3800, ROTATION_2_CAL_PARAMS};

static bool screen_on = true, connected_to_wifi = false;
extern volatile bool update_time;
volatile bool button_pressed = false;


void idle();
void draw_homescreen();
bool check_app_use(const uint16_t &touch_x, const uint16_t &touch_y);
void IRAM_ATTR push_button_isr();
void toggle_display();

void setup() {
	Serial.begin(115200);

	// Set all chip selects high to avoid bus contention during initialisation of each peripheral
	digitalWrite(TOUCH_CS, HIGH); // Touch controller chip select (if used)
	digitalWrite(TFT_CS, HIGH); // TFT screen chip select

	// Configure TFT
	pinMode(TFT_BL, OUTPUT);
	digitalWrite(TFT_BL, HIGH);

	tft.init();
	tft.setRotation(ROTATION);
	tft.setTouch(cal_data);

	// Configure button
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	attachInterrupt(BUTTON_PIN, push_button_isr, RISING);

	// Configure LED
	pinMode(LED_PIN, OUTPUT);

	init_gesture_sensor();

	// Init apps
	init_clock_timer();
	init_music_app();

	// Wait for initializations
	delay(500);
	draw_homescreen();
}


void loop() {
	if (button_pressed) {
		Serial.println(esp_get_free_heap_size());
		button_pressed = false;
		toggle_display();
	}

	if (!screen_on && get_proximity_flag()) {
		set_proximity_flag(false);
		toggle_display();
	}

	if (screen_on && get_gesture_flag()) {
		uint8_t gesture = get_gesture();
		set_gesture_flag(false);

		if (gesture == LEFT)
			toggle_display();
	}


	if (!screen_on) {
		idle();
		return;
	}


	if (update_time) {
		update_time = false;
		draw_clock_time();
	}

	uint16_t touch_x, touch_y;
	bool touch = tft.getTouch(&touch_x, &touch_y);

	if (!touch)
		return;

	// If user exited app, redraw homescreen
	if (check_app_use(touch_x, touch_y))
		draw_homescreen();
}


void idle()
{
	// increase the LED brightness
	for(int dutyCycle = 0; dutyCycle <= 128 && !button_pressed; dutyCycle++){   
		// changing the LED brightness with PWM
		analogWrite(LED_PIN, dutyCycle);
		delay(10);
	}

	// decrease the LED brightness
	for(int dutyCycle = 128; dutyCycle >= 0 && !button_pressed; dutyCycle--){
		// changing the LED brightness with PWM
		analogWrite(LED_PIN, dutyCycle);
		delay(10);
	}

	// Turn off the LED
	analogWrite(LED_PIN, 0);
}


void draw_homescreen()
{
	// Draw background color
	tft.fillRect(BACKGROUND_RECT, default_4bit_palette[background_color_index]);

	// Draw weather app
	tft.fillRect(23, 23, 196, 88, TFT_BLACK);
	tft.drawRect(WEATHER_APP_POSITION, WEATHER_BUTTON_SIZE, TFT_WHITE);
	tft.drawRect(28, 28, 186, 78, TFT_WHITE);

	// Draw apps background
	tft.fillRect(GALLERY_APP_BUTTON, TFT_BLACK);
	tft.fillRect(MUSIC_APP_BUTTON, TFT_BLACK);
	tft.fillRect(PAINT_APP_BUTTON, TFT_BLACK);

	tft.fillRect(ALARM_APP_BUTTON, TFT_BLACK);
	tft.fillRect(RGB_APP_BUTTON, TFT_BLACK);
	tft.fillRect(INFO_APP_BUTTON, TFT_BLACK);

	#ifndef NO_IMG
	// Draw apps
	tft.drawBitmap(GALLERY_APP_POSITION, gallery_app_icon, BUTTON_SIZE, TFT_WHITE);
	tft.drawBitmap(MUSIC_APP_POSITION, music_app_icon, BUTTON_SIZE, TFT_WHITE);
	tft.drawBitmap(PAINT_APP_POSITION, paint_app_icon, BUTTON_SIZE, TFT_WHITE);

	tft.drawBitmap(ALARM_APP_POSITION, alarm_app_icon, BUTTON_SIZE, TFT_WHITE);
	tft.drawBitmap(RGB_APP_POSITION, todo_app_icon, BUTTON_SIZE, TFT_WHITE);
	tft.drawBitmap(INFO_APP_POSITION, info_app_icon, BUTTON_SIZE, TFT_WHITE);
	#endif

	// Draw taskbar
	tft.fillRect(TASKBAR_RECT, TFT_BLACK);

	// Draw home logo
	tft.fillRect(7, 282, HOME_SQUARE_SIZE, HOME_SQUARE_SIZE, TFT_WHITE);
	tft.fillRect(25, 282, HOME_SQUARE_SIZE, HOME_SQUARE_SIZE, TFT_WHITE);
	tft.fillRect(7, 300, HOME_SQUARE_SIZE, HOME_SQUARE_SIZE, TFT_WHITE);
	tft.fillRect(25, 300, HOME_SQUARE_SIZE, HOME_SQUARE_SIZE, TFT_WHITE);

	// Draw time
	draw_clock_time();

	// Update weather
	update_weather(connected_to_wifi, false);
}


bool check_app_use(const uint16_t &touch_x, const uint16_t &touch_y)
{
	if (is_button_pressed(WEATHER_APP_BUTTON, touch_x, touch_y)){
		update_weather(connected_to_wifi, true);
		return true;
	} else if (is_button_pressed(GALLERY_APP_BUTTON, touch_x, touch_y)){
		galery_app(connected_to_wifi);
		return true;
	} else if (is_button_pressed(MUSIC_APP_BUTTON, touch_x, touch_y)){
		music_app();
		return true;
	} else if (is_button_pressed(PAINT_APP_BUTTON, touch_x, touch_y)){
		paint_app();
		return true;
	} else if (is_button_pressed(RGB_APP_BUTTON, touch_x, touch_y)) {
		background_color_index++;
		background_color_index %= RGB_COLOR_COUNT;

		// Wait for next color change
		delay(500);

		return true;
	} else if (is_button_pressed(INFO_APP_BUTTON, touch_x, touch_y)) {
		info_app(connected_to_wifi);
		return true;
	}

	return false;
}


void toggle_display()
{
	screen_on = !screen_on;
	digitalWrite(TFT_BL, screen_on);
}

void IRAM_ATTR push_button_isr()
{
	button_pressed = true;
}

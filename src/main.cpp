#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
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
#include "chat/chat_app.h"
#include "remote_control/remote_controller.h"


TFT_eSPI tft = TFT_eSPI();


static uint16_t cal_data[] = {300, 3500, 128, 3800, ROTATION_2_CAL_PARAMS};
static uint32_t background_color_index = 0;
static bool screen_on = true, connected_to_wifi = false;
extern volatile bool update_time;
volatile bool button_pressed = false;


void idle();
void draw_homescreen();
bool check_app_use(const uint16_t &touch_x, const uint16_t &touch_y);
void handle_gesture();
void toggle_display();

void IRAM_ATTR push_button_isr();


void setup() {
	Serial.begin(115200);

	digitalWrite(TOUCH_CS, HIGH);
	digitalWrite(TFT_CS, HIGH);

	// Configure TFT
	pinMode(TFT_BL, OUTPUT);
	digitalWrite(TFT_BL, HIGH);

	tft.init();
	tft.setRotation(ROTATION);
	tft.setTouch(cal_data);

	// Configure button
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), push_button_isr, RISING);

	// Configure LED for PWM
	pinMode(LED_PIN, OUTPUT);
	ledcSetup(0, 8000, 8);

	init_gesture_sensor();

	init_wifi();

	// Init apps
	init_clock_timer();
	init_music_app();

	// Wait for initializations
	delay(500);
	draw_homescreen();
}


void loop() {
	// If button is pressed, toggle the display
	if (button_pressed) {
		button_pressed = false;
		toggle_display();
	}

	// Check and handle any gesture
	if (get_sensor_flag())
		handle_gesture();

	// If the screen is off, return
	if (!screen_on) {
		idle();
		return;
	}

	// If the clock timer updated the time, draw the new time
	if (update_time) {
		update_time = false;
		draw_clock_time();
	}

	// Check for any touch
	uint16_t touch_x = 0, touch_y = 0;
	bool touch = tft.getTouch(&touch_x, &touch_y);

	if (!touch)
		return;

	// If user exited app, redraw homescreen
	if (check_app_use(touch_x, touch_y))
		draw_homescreen();
}


void idle()
{
	for(int dutyCycle = 0; dutyCycle <= 32 && !button_pressed && !get_sensor_flag(); dutyCycle++){   
		analogWrite(LED_PIN, dutyCycle);
		delay(40);
	}

	for(int dutyCycle = 32; dutyCycle >= 0 && !button_pressed && !get_sensor_flag(); dutyCycle--){
		analogWrite(LED_PIN, dutyCycle);
		delay(40);
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

	tft.fillRect(CHAT_APP_BUTTON, TFT_BLACK);
	tft.fillRect(RGB_APP_BUTTON, TFT_BLACK);
	tft.fillRect(INFO_APP_BUTTON, TFT_BLACK);

	// Draw apps
	tft.drawBitmap(GALLERY_APP_POSITION, gallery_app_icon, BUTTON_SIZE, TFT_WHITE);
	tft.drawBitmap(MUSIC_APP_POSITION, music_app_icon, BUTTON_SIZE, TFT_WHITE);
	tft.drawBitmap(PAINT_APP_POSITION, paint_app_icon, BUTTON_SIZE, TFT_WHITE);

	tft.drawBitmap(CHAT_APP_POSITION, chat_app_icon, BUTTON_SIZE, TFT_WHITE);
	tft.drawBitmap(RGB_APP_POSITION, rgb_app_icon, BUTTON_SIZE, TFT_WHITE);
	tft.drawBitmap(INFO_APP_POSITION, info_app_icon, BUTTON_SIZE, TFT_WHITE);

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
	} else if (is_button_pressed(MUSIC_APP_BUTTON, touch_x, touch_y)) {
		music_app();
		return true;
	} else if (is_button_pressed(PAINT_APP_BUTTON, touch_x, touch_y)) {
		paint_app();
		return true;
	} else if (is_button_pressed(CHAT_APP_BUTTON, touch_x, touch_y)) {
		chat_app(connected_to_wifi, default_4bit_palette[background_color_index]);
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


void handle_gesture()
{
	detachInterrupt(digitalPinToInterrupt(APDS9960_INT));
	reset_sensor_flag();

	uint8_t gesture = get_gesture();

	if ((screen_on && gesture == LEFT) || (!screen_on && gesture == RIGHT))
		toggle_display();

	attachInterrupt(digitalPinToInterrupt(APDS9960_INT), gesture_sensor_isr, FALLING);
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

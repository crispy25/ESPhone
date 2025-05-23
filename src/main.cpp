#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
// #include <HTTPClient.h>
#include <User_Setup.h>

#include <SparkFun_APDS9960.h>
#include <Wire.h>

#include "img.h"
#include "utils.h"
#include "macros.h"
#include "wifi/wifi.h"
#include "paint/paint_app.h"
#include "music/music_app.h"
#include "clock/clock_timer.h"
#include "weather/weather_app.h"
#include "keyboard/keyboard.h"

#define ROTATION 2
#define ROTATION_2_CAL_PARAMS 4

// #define NO_IMG 1
// #define ENABLE_I2C 1

extern volatile bool update_time;

TFT_eSPI tft = TFT_eSPI(); 

SparkFun_APDS9960 apds = SparkFun_APDS9960();


static uint32_t background_color_index = 0;
// static uint16_t cal_data[] = {354, 3684, 295, 3789, ROTATION_2_CAL_PARAMS};
static uint16_t cal_data[] = {300, 3500, 128, 3800, ROTATION_2_CAL_PARAMS};
static bool screen_on = true, connected_to_wifi = false;

volatile bool button_pressed = false;

void idle();
void draw_margins();
void draw_homescreen();
void info_app();

void IRAM_ATTR push_button_isr();


void setup() {
	Serial.begin(115200);

	#ifdef ENABLE_I2C
	Wire.setPins(SDA, SCL);
	Wire.begin();
	
	// Configure gesture sensor
	// Set interrupt pin as input
  	pinMode(APDS9960_INT, INPUT);
	// Initialize APDS-9960 (configure I2C and initial values)
	if (apds.init()) {
		Serial.println(F("APDS-9960 initialization complete"));
	} else {
		Serial.println(F("Something went wrong during APDS-9960 init!"));
	}
	
	// Start running the APDS-9960 gesture sensor engine
	if (apds.enableGestureSensor()) {
		Serial.println(F("Gesture sensor is now running"));
	} else {
		Serial.println(F("Something went wrong during gesture sensor init!"));
	}

	#endif

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

	init_clock_timer();
	init_music_app();

	// Wait for initializations
	delay(500);
	draw_homescreen();
}

void loop() {
	if (button_pressed) {
		Serial.println("Pressed");
		button_pressed = false;
		screen_on = !screen_on;
		digitalWrite(TFT_BL, screen_on);
	}

	if (!screen_on) {
		idle();
		#ifdef ENABLE_I2C
		list_devices();
		#endif
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

	// Serial.print("x=");
	// Serial.println(touch_x);
	// Serial.print("y=");
	// Serial.println(touch_y);

	if (is_button_pressed(WEATHER_APP_BUTTON, touch_x, touch_y)){
		update_weather(connect_to_wifi);
	} else if (is_button_pressed(MUSIC_APP_BUTTON, touch_x, touch_y)){
		music_app();
	} else if (is_button_pressed(PAINT_APP_BUTTON, touch_x, touch_y)){
		paint_app();
	} else if (is_button_pressed(RGB_APP_BUTTON, touch_x, touch_y)) {
		background_color_index++;
		background_color_index %= RGB_COLOR_COUNT;

		// Wait for next color change
		delay(500);

		// Update the home screen
		draw_homescreen();
	} else if (is_button_pressed(INFO_APP_BUTTON, touch_x, touch_y)) {
		info_app();
		draw_homescreen();
	}
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

	// Update weather
	update_weather();

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
}

void draw_margins()
{
  // background_color_index += 1;
  // if (background_color_index >= COLOR_COUNT)
  //   background_color_index = 0;

  // tft.drawRect(0, 0, 240, 320, all_colors[background_color_index]);
  // tft.drawRect(1, 1, 238, 318, all_colors[background_color_index]);

  // delay(500);
}


void IRAM_ATTR push_button_isr()
{
	button_pressed = true;
}


void info_app()
{
	tft.fillRect(BACKGROUND_RECT, TFT_BLACK);
	tft.fillRect(140, 288, 98, 30, TFT_BLACK);
	tft.setTextSize(2);
	tft.setCursor(20, 40);

	String user_input;
	uint16_t touch_x, touch_y;
	bool touch;

	String wifi_names[MAX_WIFIS];
	uint8_t wifi_security[MAX_WIFIS], wifi_signals[MAX_WIFIS];
	int wifis = 0;

	show_networks(wifi_names, wifi_signals, wifi_security, wifis, true);

	while (!is_button_pressed(HOME_BUTTON, touch_x, touch_y)) {

		bool touch = tft.getTouch(&touch_x, &touch_y, 100);

		if (!touch)
			continue;

		if (touch_y > TASKBAR_UPPER_LIMIT) {
			show_networks(wifi_names, wifi_signals, wifi_security, wifis, true);
		} else if (touch_y > 20) {
			uint16_t wifi_selected = (touch_y - 40) / 20;

			if (wifi_selected < wifis) {
				Serial.print(wifi_selected);

				user_input = keyboard_mode();
				
				connected_to_wifi = connect_to_wifi(wifi_names[wifi_selected], user_input);
				
				show_wifi_status(wifi_names[wifi_selected], connected_to_wifi);

				delay(3000);

				if (!connected_to_wifi)
					show_networks(wifi_names, wifi_signals, wifi_security, wifis, true);
				else {
					sync_clock_timer(NTP_SERVER);
					break;
				}
			}

		}

	}
}

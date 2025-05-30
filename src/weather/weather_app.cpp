#include "weather_app.h"

#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <string.h>

#include "img.h"
#include "api_keys.h"
#include "../keyboard/keyboard.h"

#define WEATHER_SERVER "http://api.weatherapi.com/v1/current.json?key=%s&q=%s"
#define DEFAULT_LOCATION "Bucharest"
#define WEATHER_ICON_SIZE 56, 56
#define WEATHER_ICON_POSITION 144, 40
#define UPDATE_INTERVAL 60 * 1000

extern TFT_eSPI tft;

static String prev_location = DEFAULT_LOCATION;
static String location = DEFAULT_LOCATION;


WeatherInfo parseJSON(const String &json)
{
	WeatherInfo weather_info = {};

	uint16_t temp_start_index = json.lastIndexOf("temp_c") + 8;
	uint16_t temp_end_index = json.indexOf(",", temp_start_index);
	weather_info.temperature = json.substring(temp_start_index, temp_end_index);

	uint16_t cloud_start_index = json.lastIndexOf("cloud") + 7;
	weather_info.clouds = json.charAt(cloud_start_index) != '0';

	uint16_t precip_start_index = json.lastIndexOf("precip_mm") + 11;
	uint16_t precip_end_index = json.indexOf(",", precip_start_index);
	weather_info.raining = json.substring(precip_start_index, precip_end_index).compareTo("0.0");

	return weather_info;
}


void draw_weather_stats(const WeatherInfo stats)
{
	// Clear old weather
	tft.fillRect(29, 29, 81, 76, TFT_BLACK);

	if (stats.raining)
		tft.drawBitmap(WEATHER_ICON_POSITION, raining_icon, WEATHER_ICON_SIZE, TFT_WHITE);
	else if (stats.clouds)
		tft.drawBitmap(WEATHER_ICON_POSITION, cloud_icon, WEATHER_ICON_SIZE, TFT_WHITE);
	else
		tft.drawBitmap(WEATHER_ICON_POSITION, sun_icon, WEATHER_ICON_SIZE, TFT_WHITE);

	// Print temperature on display
	tft.setTextColor(TFT_WHITE);  
	tft.setTextSize(2);
	tft.setCursor(38, 68);

	tft.print(stats.temperature);

	tft.setTextSize(1);
	tft.setCursor(tft.getCursorX(), 62);
	tft.print("o");

	tft.setCursor(tft.getCursorX(), 68);
	tft.setTextSize(1);
	tft.print("C");

	// Print location
	tft.setCursor(35, 42);
	tft.setTextSize(2);
	tft.print(location);
}


void update_weather(bool connected_to_wifi, bool ask_location)
{
	static WeatherInfo stats = {.temperature="20.0", .clouds=true, .raining=true};
	static unsigned long last_update = 0;
	static bool offline = true;

	if (!connected_to_wifi) {
		draw_weather_stats(stats);
		return;
	}


	if (ask_location){
		String user_input = keyboard_mode("Enter city:");

		if (!location.isEmpty()) {
			location = user_input;
			location.toLowerCase();
			location[0] = location[0] >= 'a' ? location[0] - 32 : location[0];
		}
	}

	if (!ask_location && millis() - last_update < UPDATE_INTERVAL && connected_to_wifi == !offline) {
		draw_weather_stats(stats);
		return;
	}

	offline = false;

	char url[128] = {0};
	sprintf(url, WEATHER_SERVER , WEATHER_API_KEY, location);

	HTTPClient http_client;
	http_client.begin(url);

	int response_code = http_client.GET();
	if (response_code == HTTP_CODE_OK) {
	  String payload = http_client.getString();
	  stats = parseJSON(payload);
	  prev_location = location;
	} else {
		location = prev_location;
	}

	last_update = millis();

	draw_weather_stats(stats);

	http_client.end();
}
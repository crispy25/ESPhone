#include "weather_app.h"

#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <string.h>

#include "img.h"
#include "api_keys.h"
#include "../keyboard/keyboard.h"

#define WEATHER_SERVER "http://api.weatherapi.com/v1/current.json?key=%s&q=%s"

extern TFT_eSPI tft;

static String location = "Bucharest";


WeatherInfo parseJSON(const String &json)
{
	WeatherInfo weather_info = {};

	uint16_t temp_start_index = json.lastIndexOf("temp_c") + 8;
	uint16_t temp_end_index = json.indexOf(",", temp_start_index);
	weather_info.temperature = json.substring(temp_start_index, temp_end_index);

	uint16_t cloud_start_index = json.lastIndexOf("cloud") + 7;
	weather_info.clouds = json.charAt(cloud_start_index) != '0';

	return weather_info;
}


void draw_weather_stats(const WeatherInfo stats)
{
	// Clear old weather
	tft.fillRect(29, 29, 81, 76, TFT_BLACK);

	if (stats.clouds)
		tft.drawBitmap(94, 29, cloud_icon, 120, 76, TFT_WHITE);
	else
		tft.drawBitmap(94, 29, sun_icon, 120, 76, TFT_WHITE);

	// Print temperature on display
	tft.setTextColor(TFT_WHITE);  
	tft.setTextSize(2);
	tft.setCursor(38, 64);
	tft.setTextFont(1);

	tft.print(stats.temperature);

	tft.setTextSize(1);
	tft.setCursor(tft.getCursorX(), 58);
	tft.print("o");

	tft.setCursor(tft.getCursorX(), 64);
	tft.setTextSize(1);
	tft.print("C");

	// Print location
	tft.setCursor(35, 44);
	tft.setTextSize(1);
	tft.print(location);
	tft.setTextFont(0);

}


void update_weather(bool connected_to_wifi, bool ask_location)
{

	static WeatherInfo stats = {.temperature="20.0", .clouds=true};

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

	char url[128] = {0};
	sprintf(url, WEATHER_SERVER , WEATHER_API_KEY, location);

	HTTPClient http_client;
	http_client.begin(url);

	int response_code = http_client.GET();
	if (response_code == HTTP_CODE_OK) {
	  String payload = http_client.getString();
	  stats = parseJSON(payload);
	}

	draw_weather_stats(stats);

	http_client.end();
}
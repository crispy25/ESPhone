#include "weather_app.h"

#include <string.h>
// #include <HTTPClient.h>
#include <TFT_eSPI.h>

#include "img.h"

#define WEATHER_SERVER "http://api.weatherapi.com/v1/current.json?key=0d2b43c8879b45d1bef103451251305&q=Bucharest"

extern TFT_eSPI tft;


WeatherInfo parseJSON(const String &json)
{
	WeatherInfo weather_info = {};

	uint16_t temp_start_index = json.lastIndexOf("temp_c") + 9;
	uint16_t temp_end_index = json.indexOf(",", temp_start_index) - 1;
	weather_info.temperature = json.substring(temp_start_index, temp_end_index);

	uint16_t cloud_start_index = json.lastIndexOf("cloud") + 8;
	weather_info.clouds = json.charAt(cloud_start_index) != '0';

	return weather_info;
}


void draw_weather_stats(const WeatherInfo stats)
{
	// Clear old weather
	tft.fillRect(29, 29, 81, 76, TFT_BLACK);

	if (stats.clouds)
	tft.drawBitmap(90, 29, cloud_icon, 120, 76, TFT_WHITE);
	else
		tft.drawBitmap(90, 29, sun_icon, 120, 76, TFT_WHITE);

	// Print temperature on display
	tft.setTextColor(TFT_WHITE);  
	tft.setTextSize(2);
	tft.setCursor(48, 64);

	tft.print(stats.temperature);

	tft.setTextSize(1);
	tft.setCursor(tft.getCursorX(), 58);
	tft.print("o");

	tft.setCursor(tft.getCursorX(), 64);
	tft.setTextSize(2);
	tft.print("C");
}


void update_weather()
{

	WeatherInfo stats = {.temperature="20", .clouds=true};

	draw_weather_stats(stats);

	// TODO
	// HTTPClient http_client;
	// http_client.begin(WEATHER_SERVER);

	// int response_code = http_client.GET();
	// if (response_code == HTTP_CODE_OK) {
	//   String payload = http_client.getString();
	//   payload = payload.substring(272, 276);;

		
	// }

	// http_client.end();
}
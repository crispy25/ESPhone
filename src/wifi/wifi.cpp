#include "wifi.h"

#include <TFT_eSPI.h>
#include <WiFi.h>

#include "../utils/utils.h"
#include "../keyboard/keyboard.h"
#include "../clock/clock_timer.h"

extern TFT_eSPI tft;

int scan_networks(String wifi_names[MAX_WIFIS], uint8_t wifi_signals[MAX_WIFIS], uint8_t wifi_security[MAX_WIFIS])
{
	int wifis = WiFi.scanNetworks();
	for (uint8_t i = 0; i < min(wifis, MAX_WIFIS); i++) {
		String wifi_name = WiFi.SSID(i).substring(0, 10);

		wifi_names[i] = wifi_name;
		wifi_signals[i] = WiFi.RSSI(i);
		wifi_security[i] = WiFi.encryptionType(i);
	}

	return wifis;
}


int show_networks(String wifi_names[MAX_WIFIS], uint8_t wifi_signals[MAX_WIFIS], uint8_t wifi_security[MAX_WIFIS], int &wifis, bool update)
{
	tft.fillRect(BACKGROUND_RECT, TFT_BLACK);
	tft.setTextSize(2);
	tft.setCursor(0, 20);
	tft.println("WIFI:");

	if (update)
		wifis = scan_networks(wifi_names, wifi_signals, wifi_security);

	for (uint8_t i = 0; i < min(wifis, MAX_WIFIS); i++) {
		tft.fillRect(1, tft.getCursorY() + 5, 15, 15, TFT_WHITE);
		tft.setCursor(22, tft.getCursorY() + 5);
		tft.println(wifi_names[i] + " (" + wifi_signals[i] + ") " + wifi_security[i]);
	}

	return wifis;
}


bool connect_to_wifi(String wifi, String password)
{
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	WiFi.begin(wifi, password);
	
	uint8_t attempts = 0;

	tft.fillRect(BACKGROUND_RECT, TFT_BLACK);
	tft.setTextSize(2);
	tft.setCursor(20, 140);
	tft.println("Connecting to");
	tft.setCursor(20, 160);
	tft.println(wifi);
	tft.setCursor(20, 180);

	while (WiFi.status() != WL_CONNECTED && attempts++ < MAX_WIFI_CONN_ATTEMPTS) {
		tft.print('.');
		delay(1000);
	}

	return WiFi.status() == WL_CONNECTED;
}


void show_wifi_status(String wifi, bool connected)
{
	tft.fillRect(BACKGROUND_RECT, TFT_BLACK);
	tft.setTextSize(2);
	tft.setCursor(20, 140);

	if (connected)
		tft.println("Connected to");
	else
		tft.println("Can't connect to");

	tft.setCursor(20, 160);
	tft.print(wifi);
}


void info_app(bool &connected_to_wifi)
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

		if (touch_y > TASKBAR_UPPER_LIMIT && touch_x > 48) {
			show_networks(wifi_names, wifi_signals, wifi_security, wifis, true);
		} else if (touch_y > 20) {
			uint16_t wifi_selected = (touch_y - 40) / 20;

			if (wifi_selected < wifis) {
				Serial.print(wifi_selected);

				user_input = keyboard_mode("Enter password:");

				if (!user_input.isEmpty() || wifi_security[wifi_selected] == 0) {
						
					connected_to_wifi = connect_to_wifi(wifi_names[wifi_selected], user_input);
					
					show_wifi_status(wifi_names[wifi_selected], connected_to_wifi);

					delay(3000);
				}

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

#include "wifi.h"

#include <TFT_eSPI.h>
#include <WiFi.h>



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

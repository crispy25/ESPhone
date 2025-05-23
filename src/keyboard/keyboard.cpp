#include "keyboard.h"

#include <TFT_eSPI.h>

#include "../macros.h"
#include "../utils.h"
#include "../img.h"

extern TFT_eSPI tft;


void show_keyboard()
{
	tft.fillRect(BACKGROUND_RECT, TFT_BLACK);

	tft.drawBitmap(4, 132, keyboard, 240, 141, TFT_WHITE);
}


void clear_text()
{
	tft.fillRect(0, 40, 240, 91, TFT_BLACK);
	tft.setCursor(20, 40);
}


String keyboard_mode(const String prompt)
{

	String user_input;
	uint16_t touch_x, touch_y;
	uint8_t key_x, key_y;
	bool touch, caps = false;

	show_keyboard();

	tft.setTextSize(2);

	tft.setCursor(20, 20);
	tft.print(prompt);

	tft.setCursor(20, 40);

	while (!is_button_pressed(HOME_BUTTON, touch_x, touch_y)) {
		bool touch = tft.getTouch(&touch_x, &touch_y, 100);

		if (!touch)
			continue;

		if (touch_y > 132 && touch_y < 250) {
			key_x = (touch_x - 6) / 23;
			key_y = (touch_y - 135) / 23;

			// Serial.println(keys[key_y * 10 + key_x]);
			char key = keys[key_y * 10 + key_x] - (caps * 32);
			user_input += key;
			tft.print(key);

			delay(500);
		} else if (touch_y > 250 && touch_y < TASKBAR_UPPER_LIMIT) {
			key_x = (touch_x - 4) / 23 / 3;

			if (!key_x)
				caps = !caps;
			else if (key_x == 1) {
				tft.setTextSize(1);
				tft.print(' ');
				tft.setTextSize(2);

				user_input += ' ';
			} else {
				return user_input;
			}

			delay(500);
		} else {
			user_input.clear();
			clear_text();
		}
	}

	return "";
}


#include <Arduino.h>
#include <Wire.h>

#include "utils.h"


bool is_button_pressed(int bx, int by, int bw, int bh, int px, int py)
{
	return bx <= px && px <= bx + bw && by <= py && py <= by + bh;
}


// minx=0, miny=259, maxx=3935, maxy=4095
void test_touch()
{
    static uint16_t min_x = -1, min_y = -1, max_x = 0, max_y = 0;
	uint16_t touch_x = 0, touch_y = 0;
	// bool touch = tft.getTouch(&touch_x, &touch_y);

	// if (!touch)
	// 	return;

	if (min_x > touch_x && touch_x) {
		min_x = touch_x;
		Serial.print("minx=");
		Serial.println(touch_x);
	}
	if (max_x < touch_x) {
		max_x = touch_x;
		Serial.print("maxx=");
		Serial.println(touch_x);
	}
	if (min_y > touch_y) {
		min_y = touch_y;
		Serial.print("miny=");
		Serial.println(touch_y);
	}
	if (max_y < touch_y) {
		max_y = touch_y;
		Serial.print("maxy=");
		Serial.println(touch_y);
	}
}

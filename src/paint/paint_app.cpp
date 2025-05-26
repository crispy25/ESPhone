#include "paint_app.h"

#include <TFT_eSPI.h>

#include "../utils/macros.h"
#include "../utils/utils.h"

#define CHANGE_COLOR_BUTTON 200, 280, 40, 40

extern TFT_eSPI tft;
extern bool button_pressed;


void paint_app()
{
	uint32_t selected_color = 0x000F;
	uint16_t touch_x = 0, touch_y = 0;
	uint8_t selected_color_index = 0, multi_color_index = 0, brush_size = 8;;
	bool touch, multi_color = false;

	// Draw white screen
	tft.fillRect(BACKGROUND_RECT, TFT_WHITE);

	// Draw selected color
	tft.fillRect(140, 288, 98, 30, TFT_BLACK);
	tft.setTextSize(2);
	tft.setCursor(124, 292);
	tft.print("Color: ");

	tft.fillRect(200, 282, 34, 34, selected_color);

	while (!is_button_pressed(HOME_BUTTON, touch_x, touch_y)) {

		if (button_pressed) {
			button_pressed = false;
			tft.fillRect(BACKGROUND_RECT, TFT_WHITE);
		}

		touch = tft.getTouch(&touch_x, &touch_y, 100);

		if (!touch)
			continue;

		if (touch_y < TASKBAR_UPPER_LIMIT - brush_size){
			tft.fillCircle(touch_x, touch_y, brush_size, selected_color);
			if (multi_color) {
				multi_color_index %= RGB_COLOR_COUNT;
				selected_color = default_4bit_palette[++multi_color_index];
			}
		} else if (is_button_pressed(CHANGE_COLOR_BUTTON, touch_x, touch_y)) {
			selected_color_index++;
			if (selected_color_index == RGB_COLOR_COUNT) {
				multi_color = true;

				tft.fillRect(200, 282, 34, 34, TFT_BLACK);
				tft.setTextSize(3);
				tft.setCursor(200, 288);
				tft.print("M");

				selected_color_index = 0;
				delay(1000);
			} else {
				multi_color = false;
				selected_color = default_4bit_palette[selected_color_index];
				tft.fillRect(200, 282, 34, 34, selected_color);
			}
			delay(100);
		}
	}
}
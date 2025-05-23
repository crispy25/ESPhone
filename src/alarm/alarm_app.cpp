#include "alarm_app.h"

#include <TFT_eSPI.h>

#include "../macros.h"
#include "../utils.h"

extern TFT_eSPI tft;

static uint32_t time_remaining = 0;

void set_alarm(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    time_remaining = hours * 3600 + minutes * 60 + seconds;
    // TODO: Init timer
}

void alarm_app()
{
    uint16_t touch_x = 0, touch_y = 0;
    bool touch = false;

    while (!is_button_pressed(HOME_BUTTON, touch_x, touch_y)) {

		touch = tft.getTouch(&touch_x, &touch_y, 100);

		if (!touch)
			continue;
    }
}

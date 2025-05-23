#include <Arduino.h>
#include <Wire.h>

#include "utils.h"

// uint32_t rgb_colors[RGB_COLOR_COUNT] = {0x0000, 0x000F, 0x03E0, 0x03EF, 0x7800, 0x780F, 0x7BE0, 0xD69A, 0x7BEF, 0x001F, 0x07E0, 0x07FF, 0xF800, 0xF81F, 0xFFE0, 0xFFFF, 0xFDA0, 0xB7E0, 0xFE19, 0x9A60, 0xFEA0, 0xC618, 0x867D, 0x915C};

bool is_button_pressed(int bx, int by, int bw, int bh, int px, int py)
{
	return bx <= px && px <= bx + bw && by <= py && py <= by + bh;
}


// minx=0, miny=259, maxx=3935, maxy=4095
void test_touch()
{
    static uint16_t min_x = -1, min_y = -1, max_x = 0, max_y = 0;
	uint16_t touch_x, touch_y;
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


void list_devices() {
	byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(2000);           // wait 5 seconds for next scan
}

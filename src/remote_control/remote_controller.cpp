#include "remote_controller.h"

#include <Wire.h>
#include <SparkFun_APDS9960.h>

#include "../utils/utils.h"
#include "../utils/macros.h"


static SparkFun_APDS9960 apds = SparkFun_APDS9960();

static volatile bool gesture_flag = false;
static volatile bool proximity_flag = false;


void IRAM_ATTR gesture_isr()
{
	gesture_flag = true;
}


void IRAM_ATTR proximity_isr()
{
	proximity_flag = true;
}


bool get_gesture_flag()
{
    return gesture_flag;
}


bool get_proximity_flag()
{
    return proximity_flag;
}

bool set_gesture_flag(bool value)
{
    gesture_flag = value;
}


bool set_proximity_flag(bool value)
{
    proximity_flag = value;
}


void init_gesture_sensor()
{
	Wire.setPins(SDA, SCL);
	Wire.begin();
	
	// Configure gesture sensor
	// Set interrupt pin as input
  	pinMode(APDS9960_INT, INPUT);
	// Initialize APDS-9960 (configure I2C and initial values)
	if (apds.init()) {
		Serial.println(F("APDS-9960 initialization complete"));
	} else {
		Serial.println(F("Something went wrong during APDS-9960 init!"));
	}
	
	// Start running the APDS-9960 gesture sensor engine
	if (apds.enableGestureSensor()) {
		Serial.println(F("Gesture sensor is now running"));
	} else {
		Serial.println(F("Something went wrong during gesture sensor init!"));
	}
	
	// Start running the APDS-9960 proximity sensor
	if (apds.enableProximitySensor(true)) {
		Serial.println(F("Proximity sensor is now running"));
	} else {
		Serial.println(F("Something went wrong during proximity sensor init!"));
	}
}


uint8_t get_gesture()
{
    if (apds.isGestureAvailable()) {
    switch (apds.readGesture()) {
      case DIR_UP:
        Serial.println("UP");
		return 0;
      case DIR_DOWN:
        Serial.println("DOWN");
		return 1;
      case DIR_LEFT:
        Serial.println("LEFT");
		return 2;
      case DIR_RIGHT:
        Serial.println("RIGHT");
		return 3;
      case DIR_NEAR:
        Serial.println("NEAR");
		return 4;
      case DIR_FAR:
        Serial.println("FAR");
		return 5;
      default:
        Serial.println("NONE");
		return -1;
    }
  }
}

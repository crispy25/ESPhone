#include "remote_controller.h"

#include <Wire.h>
#include <SparkFun_APDS9960.h>

#include "../utils/utils.h"
#include "../utils/macros.h"


static SparkFun_APDS9960 apds = SparkFun_APDS9960();
static volatile bool sensor_flag = false;


void IRAM_ATTR gesture_sensor_isr()
{
	sensor_flag = true;
}


bool get_sensor_flag()
{
    return sensor_flag;
}


void reset_sensor_flag()
{
    sensor_flag = false;
}


void init_gesture_sensor()
{
	Wire.setPins(SDA, SCL);
	Wire.begin();
	
	attachInterrupt(digitalPinToInterrupt(APDS9960_INT), gesture_sensor_isr, FALLING);

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
	
	// // Start running the APDS-9960 proximity sensor
	// if (apds.enableProximitySensor(false)) {
	// 	Serial.println(F("Proximity sensor is now running"));
	// } else {
	// 	Serial.println(F("Something went wrong during proximity sensor init!"));
	// }
}


uint8_t get_gesture()
{
    if (apds.isGestureAvailable()) {
    switch (apds.readGesture()) {
      case DIR_UP:
        Serial.println("UP");
		return UP;
      case DIR_DOWN:
        Serial.println("DOWN");
		return DOWN;
      case DIR_LEFT:
        Serial.println("LEFT");
		return LEFT;
      case DIR_RIGHT:
        Serial.println("RIGHT");
		return RIGHT;
      case DIR_NEAR:
        Serial.println("NEAR");
		return NEAR;
      case DIR_FAR:
        Serial.println("FAR");
		return FAR;
      default:
        Serial.println("NONE");
		return -1;
    }
  }
}

uint8_t get_proximity_level()
{	
	uint8_t val = 0xFF;
    apds.readProximity(val);
	return val;
}
